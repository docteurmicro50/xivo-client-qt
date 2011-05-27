/* XiVO Client
 * Copyright (C) 2007-2011, Proformatique
 *
 * This file is part of XiVO Client.
 *
 * XiVO Client is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version, with a Section 7 Additional
 * Permission as follows:
 *   This notice constitutes a grant of such permission as is necessary
 *   to combine or link this software, or a modified version of it, with
 *   the OpenSSL project's "OpenSSL" library, or a derivative work of it,
 *   and to copy, modify, and distribute the resulting work. This is an
 *   extension of the special permission given by Trolltech to link the
 *   Qt code with the OpenSSL library (see
 *   <http://doc.trolltech.com/4.4/gpl.html>). The OpenSSL library is
 *   licensed under a dual license: the OpenSSL License and the original
 *   SSLeay license.
 *
 * XiVO Client is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with XiVO Client.  If not, see <http://www.gnu.org/licenses/>.
 */

/* $Revision$
 * $Date$
 */

#include <QDebug>
#include <QLabel>
#include <QVBoxLayout>
#include <QTableWidget>

#include "popcaastra.h"
#include "../ui_popcaastra.h"
#include "userinfo.h"
#include "channelinfo.h"
#include "aastrasipnotify.h"
#include "xivoconsts.h"
#include "incomingwidget.h"

#define MAX_LINES 4

PopcAastra::PopcAastra(QWidget *parent)
    : XLet(parent), m_ui(new Ui::PopcAastra)
{
    m_ui->setupUi(this);
    setTitle(tr("POPC Aastra operator"));

    m_calls_list = new QVBoxLayout(m_ui->m_calls_layout);
    m_destinations_list = new QVBoxLayout(m_ui->m_destinations_layout);

    m_timerid = 0;
    m_deltasec = 1;
    startTimer(1000);

    // Signals / slots
    connect(b_engine, SIGNAL(monitorPeer(UserInfo *)),
            this, SLOT(monitorPeer(UserInfo *)));
    connect(b_engine, SIGNAL(updateUserConfig(const QString &)),
            this, SLOT(updateUserConfig(const QString &)));
    connect(b_engine, SIGNAL(updateUserStatus(const QString &)),
            this, SLOT(updateUserStatus(const QString &)));
    connect(b_engine, SIGNAL(updatePhoneConfig(const QString &)),
            this, SLOT(updatePhoneConfig(const QString &)));
    connect(b_engine, SIGNAL(updatePhoneStatus(const QString &)),
            this, SLOT(updatePhoneStatus(const QString &)));
    connect(b_engine, SIGNAL(updateChannelStatus(const QString &)),
            this, SLOT(updateChannelStatus(const QString &)));
    connect(b_engine, SIGNAL(broadcastNumberSelection(const QStringList &)),
            this, SLOT(receiveNumberSelection(const QStringList &)));

    connect(m_ui->btn_vol_up, SIGNAL(clicked()), this, SLOT(volUp()));
    connect(m_ui->btn_vol_down, SIGNAL(clicked()), this, SLOT(volDown()));
    connect(m_ui->btn_right, SIGNAL(clicked()), this, SLOT(navRight()));
    connect(m_ui->btn_hangup, SIGNAL(clicked()), this, SLOT(hangup()));
    connect(m_ui->btn_prg_1, SIGNAL(clicked()), this, SLOT(prgkey1()));
}

/*! \brief update display according to call list
 *
 * Read m_calllist and update m_afflist accordingly.
 */
void PopcAastra::updateDisplay()
{
    qDebug() << Q_FUNC_INFO;
    foreach (QString key, m_incomingcalls.keys()) {
        m_incomingcalls[key]->updateWidget();
    }
}

void PopcAastra::updateChannelStatus(const QString & xchannel)
{
    //qDebug() << Q_FUNC_INFO << xchannel;
    removeDefunctWidgets();
    const ChannelInfo * channelinfo = b_engine->channels().value(xchannel);
    if (channelinfo == NULL) {
        qDebug() << Q_FUNC_INFO << "null chaninfo";
        return;
    }

    const UserInfo * me = b_engine->user(b_engine->getFullId());
    if (me == NULL) {
        qDebug() << Q_FUNC_INFO << " Null user";
        return;
    }
    
    bool ismine = false;
    foreach (QString xphone, b_engine->iterover("phones").keys()) {
        const PhoneInfo * info = b_engine->phone(xphone);
        if (info->xchannels().contains(xchannel)
            && b_engine->phonenumbers(me).contains(info->number())) {
            ismine = true;
        }
    }
    if (! ismine) return;

    // Check if this channel is already tracked
    if (m_incomingcalls.contains(xchannel)) {
        // Already tracked
        //qDebug() << Q_FUNC_INFO << " Old channel updating";
        //qDebug() << Q_FUNC_INFO << " commstatus " << channelinfo->commstatus();
    } else {
        // New channel
        int guessedline = findFirstAvailableLine();
        //qDebug() << Q_FUNC_INFO << "The guessed line number for this call is " << guessedline;
        IncomingWidget * newcall = new IncomingWidget(guessedline, xchannel, this);
        m_incomingcalls[xchannel] = newcall;
        m_calls_list->addWidget(newcall);
        connect(newcall, SIGNAL(doHangUp(int)), this, SLOT(hupline(int)));
        connect(newcall, SIGNAL(doBlindTransfer(int, const QString &, const QString &)),
                        this, SLOT(blindTransfer(int, const QString &, const QString &)));
        connect(newcall, SIGNAL(doAttendedTransfer(int)), this, SLOT(attendedTransfer(int)));
        connect(newcall, SIGNAL(selectLine(int)), this, SLOT(selectLine(int)));
    }
    IncomingWidget * current = m_incomingcalls[xchannel];
    current->updateWidget();
}

/*! \brief check for defunct channels that are still shown in the call and transfer lists */
void PopcAastra::removeDefunctWidgets()
{
    //qDebug() << Q_FUNC_INFO;
    if (m_incomingcalls.size() == 0 && m_transferedcalls.size() == 0) return;
    const QHash<QString, ChannelInfo *> & channels = b_engine->channels();
    foreach (const QString channel, m_incomingcalls.keys()) {
        if (! channels.contains(channel)) {
            delete m_incomingcalls[channel];
            m_incomingcalls.remove(channel);
        }
    }
    foreach (const QString phone, m_transferedcalls.keys()) {
        if (! b_engine->hasPhone(phone) || m_transferedcalls[phone]->readyToBeRemoved()) {
            delete m_transferedcalls[phone];
            m_transferedcalls.remove(phone);
        }
    }
}

/*! \brief prints the content of m_incomingcalls */
void PopcAastra::debugIncomingCalls() const
{
    qDebug() << Q_FUNC_INFO << "Number of incoming calls " << m_incomingcalls.size();
    for (QHash<QString, IncomingWidget *>::const_iterator i = m_incomingcalls.constBegin();
        i != m_incomingcalls.constEnd(); ++i) {
        IncomingWidget * current = i.value();
        qDebug() << Q_FUNC_INFO << i.key() << current->toString();
    }
}

/*! \brief finds the first line available to place this channel
 *  The goal is to know/guess on which line a call is to avoid
 *  mistakes when doing operations to a call since we are dealing
 *  with lines on the phone device when using aastra xml api
 *  Return -1 when all the lines are taken
 */
int PopcAastra::findFirstAvailableLine() const {
    if (m_incomingcalls.size() == 0) {
        return 1;
    }
    for (int i = 0; i < MAX_LINES; i++) {
        bool free = true;
        foreach (QString channel, m_incomingcalls.keys()) {
            if (m_incomingcalls[channel]->line() == i + 1) free = false;
        }
        if (free) return i + 1;
    }
    return -1;
}

/*! \brief parse new phone status
 * 0 = Available
 * 1 = Talking
 * 4 = Not available
 * 8 = Ringing
 * 9 = (On line OR calling) AND ringing
 * 16 = On hold
 */
void PopcAastra::updatePhoneStatus(const QString & xphoneid)
{
    //qDebug() << Q_FUNC_INFO << xphoneid;
    removeDefunctWidgets();
    const PhoneInfo * phone = b_engine->phone(xphoneid);
    if (phone == NULL) return;

    foreach (QString xchannel, phone->xchannels()) {
        if (m_transferedcalls.contains(xchannel)) {
            TransferedWidget * w = m_transferedcalls[xchannel];
            w->updateWidget();
        }
    }
}

void PopcAastra::timerEvent(QTimerEvent * event)
{
    //qDebug() << Q_FUNC_INFO;
    foreach (QString key, m_incomingcalls.keys())
        m_incomingcalls[key]->updateWidget();
    foreach (QString key, m_transferedcalls.keys())
        m_transferedcalls[key]->updateWidget();
    if (m_transferedcalls.size() || m_incomingcalls.size())
        removeDefunctWidgets();
}

void PopcAastra::hupline(int line)
{
    //qDebug() << Q_FUNC_INFO << line;
    QList<QString> commands;
    commands.append(getKeyUri(LINE, line));
    commands.append(getKeyUri(GOODBYE));
    emit ipbxCommand(getAastraSipNotify(commands, SPECIAL_ME));
}

/*! \brief blind transfer the call on the line to the number in the name/number field */
void PopcAastra::blindTransfer(int line, const QString & transferedname, 
        const QString & transferednumber)
{
    //qDebug() << Q_FUNC_INFO << line;
    QList<QString> commands;
    commands.append(getKeyUri(LINE, line));
    commands.append(getKeyUri(XFER));
    QString number = m_ui->txt_number_name->text();
    for (int i = 0; i < number.size(); ++i) {
        const QChar c = number[i];
        if (c.isDigit()) {
            commands.append(getKeyUri(KEYPAD, c.digitValue()));
        }
    }
    commands.append(getKeyUri(XFER));
    trackTransfer(number, transferedname, transferednumber);
    emit ipbxCommand(getAastraSipNotify(commands, SPECIAL_ME));
}

/*! \brief starts tracking a number after a transfer */
void PopcAastra::trackTransfer(QString number, const QString & tname, const QString & tnum)
{
    qDebug() << Q_FUNC_INFO << number;
    foreach (QString id, b_engine->iterover("phones").keys()) {
        const PhoneInfo * p = b_engine->phone(id);
        if (p->number() == number) {
            TransferedWidget * w = new TransferedWidget(number, tname, tnum, this);
            m_transferedcalls[id] = w;
            m_destinations_list->addWidget(w);
            connect(
                w, SIGNAL(intercept(const QString &)),
                this, SLOT(doIntercept(const QString &)));
            break;
        }
    }
}

/*! \brief attended transfer to the line in the number/name field */
void PopcAastra::attendedTransfer(int line)
{
    qDebug() << Q_FUNC_INFO << line;
    QList<QString> commands;
    commands.append(getKeyUri(LINE, line));
    commands.append(getKeyUri(XFER));
    QString number = m_ui->txt_number_name->text();
    for (int i = 0; i < number.size(); ++i) {
        const QChar c = number[i];
        if (c.isDigit())
            commands.append(getKeyUri(KEYPAD, c.digitValue()));
    }
    commands.append(getKeyUri(NAV_RIGHT));
    emit ipbxCommand(getAastraSipNotify(commands, SPECIAL_ME));
}

/*! \brief intercept a call using *8 exten */
void PopcAastra::doIntercept(const QString & number)
{
    qDebug() << Q_FUNC_INFO << number;
    QList<QString> commands;
    commands.append(getKeyUri(KEYPAD_STAR));
    for (int i = 0; i < number.size(); ++i) {
        const QChar c = number[i];
        if (c.isDigit())
            commands.append(getKeyUri(KEYPAD, c.digitValue()));
    }
    commands.append(getKeyUri(NAV_RIGHT));
    emit ipbxCommand(getAastraSipNotify(commands, SPECIAL_ME));
}

/*! \brief select the line */
void PopcAastra::selectLine(int line)
{
    qDebug() << Q_FUNC_INFO << line;
    emit ipbxCommand(getAastraKeyNotify(LINE, SPECIAL_ME, line));
}

/*! \brief park the call on line line */
void PopcAastra::parkcall(int line)
{
    qDebug() << Q_FUNC_INFO << line;
}

void PopcAastra::updatePhoneConfig(const QString & phone)
{
    qDebug() << Q_FUNC_INFO << phone;
}

void PopcAastra::updateUserConfig(const QString & xuserid)
{
    qDebug() << Q_FUNC_INFO << xuserid;
    updateDisplay();
}

void PopcAastra::updateUserStatus(const QString & xuserid)
{
    qDebug() << Q_FUNC_INFO << xuserid;
    updateDisplay();
}

/*! \brief turns the volume up */
void PopcAastra::volUp()
{
    qDebug() << Q_FUNC_INFO;
    emit ipbxCommand(getAastraKeyNotify(VOL_UP, SPECIAL_ME));
}

/*! \brief turns the volume down */
void PopcAastra::volDown()
{
    qDebug() << Q_FUNC_INFO;
    emit ipbxCommand(getAastraKeyNotify(VOL_DOWN, SPECIAL_ME));
}

/*! \brief press the navigation right button of the device */
void PopcAastra::navRight()
{
    qDebug() << Q_FUNC_INFO;
    emit ipbxCommand(getAastraKeyNotify(NAV_RIGHT, SPECIAL_ME));
}

/*! \brief hang up the active line */
void PopcAastra::hangup()
{
    qDebug() << Q_FUNC_INFO;
    emit ipbxCommand(getAastraKeyNotify(GOODBYE, SPECIAL_ME));
}

/*! \brief simulates a press on the programmable button 1 */
void PopcAastra::prgkey1()
{
    qDebug() << Q_FUNC_INFO;
    emit ipbxCommand(getAastraKeyNotify(PRG_KEY, SPECIAL_ME, 1));
}

/*! \brief receive a list of numbers for a selected peer or contact */
void PopcAastra::receiveNumberSelection(QStringList numbers)
{
    // qDebug() << Q_FUNC_INFO;
    if (numbers.isEmpty()) {
        m_ui->txt_number_name->setText("");
    } else if (numbers.size() == 1) {
        m_ui->txt_number_name->setText(numbers.at(0));
    } else {
        // FIXME: take multiple numbers into account
        m_ui->txt_number_name->setText(numbers.at(0));
    }
}

/*! \brief destructor
 */
PopcAastra::~PopcAastra()
{
    qDebug() << Q_FUNC_INFO;
    delete m_ui;
}

