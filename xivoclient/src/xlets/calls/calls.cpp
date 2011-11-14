/* XiVO Client
 * Copyright (C) 2007-2011, Avencall
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

#include "calls.h"

Q_EXPORT_PLUGIN2(xletcallsplugin, XLetCallsPlugin);

XLet* XLetCallsPlugin::newXLetInstance(QWidget *parent)
{
    b_engine->registerTranslation(":/obj/calls_%1");
    return new XletCalls(parent);
}


XletCalls::XletCalls(QWidget *parent)
    : XLet(parent)
{
    setTitle(tr("Calls"));
    QVBoxLayout *toplayout = new QVBoxLayout(this);
    toplayout->setMargin(0);
    QLabel *titleLabel = new QLabel("                     ", this);
    toplayout->addWidget( titleLabel, 0, Qt::AlignCenter);

    QScrollArea *scrollarea = new QScrollArea(this);
    scrollarea->setWidgetResizable(true);
    QWidget *w = new QWidget(scrollarea);
    scrollarea->setWidget(w);
    m_layout = new QVBoxLayout(w);
    setObjectName("scroller");
    setAcceptDrops(true);
    m_layout->addStretch(1);
    toplayout->addWidget(scrollarea);

    // connect signals/slots
    connect(this, SIGNAL(changeTitle(const QString &)),
            titleLabel, SLOT(setText(const QString &)));

    connect(b_engine, SIGNAL(monitorPeerChanged()),
            this, SLOT(monitorPeerChanged()));
    connect(b_engine, SIGNAL(updatePhoneStatus(const QString &)),
            this, SLOT(updatePhoneStatus(const QString &)));
    connect(b_engine, SIGNAL(updateChannelStatus(const QString &)),
            this, SLOT(updateChannelStatus(const QString &)));
}

/*! \brief hang up channel
 */
void XletCalls::hupchan(const QString &hangupchan)
{
    b_engine->actionCall("hangup",
                         "chan:" + m_monitored_ui->id() + ":" + hangupchan); // Call
}

/*! \brief transfers the channel to a number
 */
void XletCalls::transftonumberchan(const QString &chan)
{
    b_engine->actionCall("transfer",
                         "chan:" + m_monitored_ui->id() + ":" + chan,
                         "ext:special:dialxlet"); // Call
}

void XletCalls::updatePhoneStatus(const QString & xphoneid)
{
    if (m_monitored_ui == NULL)
        return;
    if (! m_monitored_ui->phonelist().contains(xphoneid))
        return;
    const PhoneInfo * phoneinfo = b_engine->phone(xphoneid);
    if (phoneinfo == NULL)
        return;

    foreach (const QString xchannel, m_affhash.keys()) {
        CallWidget * callwidget = m_affhash.value(xchannel);
        if (! phoneinfo->xchannels().contains(xchannel)) {
            delete callwidget;
            m_affhash.remove(xchannel);
        }
    }
    foreach (const QString xchannel, phoneinfo->xchannels()) {
        if (! m_affhash.contains(xchannel)) {
            CallWidget * callwidget = new CallWidget(m_monitored_ui,
                                                     xchannel,
                                                     this);
            connect(callwidget, SIGNAL(doHangUp(const QString &)),
                    this, SLOT(hupchan(const QString &)));
            connect(callwidget, SIGNAL(doTransferToNumber(const QString &)),
                    this, SLOT(transftonumberchan(const QString &)));
            m_layout->insertWidget(m_layout->count() - 1, callwidget,
                                   0, Qt::AlignTop);
            m_affhash[xchannel] = callwidget;
        }
    }
}

void XletCalls::updateChannelStatus(const QString & xchannel)
{
    const ChannelInfo * channelinfo = b_engine->channels().value(xchannel);
    if (channelinfo == NULL)
        return;
    QString status = channelinfo->commstatus();
    if (status == CHAN_STATUS_HANGUP)
        return;
    if (m_affhash.contains(xchannel)) {
        m_affhash[xchannel]->updateWidget(xchannel);
    }
}

/*! \brief update display according to call list
 *
 * Read m_calllist and update m_afflist accordingly.
 */
void XletCalls::updateDisplay()
{
    if (m_monitored_ui == NULL)
        return;
    foreach (const QString phoneid, m_monitored_ui->phonelist())
        updatePhoneStatus(phoneid);
}

/*! \brief filter events based on the mimetype
 */
void XletCalls::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasFormat(XUSERID_MIMETYPE)) {
        event->acceptProposedAction();
    }
}

/*! \brief updates the peer to be monitored
 *
 * can be called from reset(), dropEvent(), or at the beginning
 * of a session
 */
void XletCalls::monitorPeerChanged()
{
    if (m_monitored_ui == NULL)
        return;
    changeTitle(tr("Monitoring : %1").arg(m_monitored_ui->fullname()));
    updateDisplay();
}

/*! \brief receive drop Events.
 *
 * check if the dropped "text" is a Peer "id"
 * and start to monitor it
 */
void XletCalls::dropEvent(QDropEvent *event)
{
    if (!event->mimeData()->hasFormat(XUSERID_MIMETYPE)) {
        event->ignore();
        return;
    }
    b_engine->monitorPeerRequest(event->mimeData()->data(XUSERID_MIMETYPE));
    event->acceptProposedAction();
}
