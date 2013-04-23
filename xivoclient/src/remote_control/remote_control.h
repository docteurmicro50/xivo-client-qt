/* XiVO Client
 * Copyright (C) 2007-2013, Avencall
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

#ifndef __REMOTE_CONTROL_H__
#define __REMOTE_CONTROL_H__

#ifdef FUNCTESTS

#include <QObject>
#include <QAbstractItemModel>
#include <string>
#include <exception>

#include <main.h>

#include "xlets/conference/conflist.h"

using namespace std;

class QLocalServer;
class QLocalSocket;

struct RemoteControlCommand {
    QString action;
    QVariantList arguments;
};

enum RemoteControlResponse {
    TEST_FAILED,
    TEST_UNKNOWN,
    TEST_PASSED
};

class TestFailedException : public exception {
    public:
        TestFailedException(const QString & message = "");
        ~TestFailedException() throw();

        virtual const char* what() const throw();

    private:
        QString message;
};

class RemoteControl : public QObject
{
    Q_OBJECT

    public:
        RemoteControl(ExecObjects);
        void pause(unsigned);
        ~RemoteControl();

    public:
        void i_stop_the_xivo_client();
        void i_go_to_the_xivo_client_configuration();
        void i_close_the_xivo_client_configuration();
        void configure(const QVariantList &);
        void i_log_in_the_xivo_client();
        void i_log_out_of_the_xivo_client();
        QVariantMap get_identity_infos();
        QVariantMap get_queue_members_infos();
        void set_queue_for_queue_members(const QVariantList &);
        QVariantMap get_sheet_infos();
        QVariantMap get_conference_room_infos();
        QVariantMap get_switchboard_infos();
        void set_search_for_directory(const QVariantList &);
        QVariantMap get_remote_directory_infos();
        void set_search_for_remote_directory(const QVariantList &);
        void exec_double_click_on_number_for_name(const QVariantList &);

    signals:
        void select_queue(const QString & queue_id);
        void itemDoubleClicked(QTableWidgetItem*);

    public slots:
        void on_error(const QString &);

    private slots:
        void newConnection();
        void processCommands();

    private:
        RemoteControlCommand parseCommand(const QByteArray & raw_command);
        void sendResponse(RemoteControlResponse response,
              QString command,
              QString message = "",
              QVariantMap return_value = QVariantMap());
        bool commandMatches(RemoteControlCommand, std::string);

        QString getValueInModel(QAbstractItemModel* model, int row, int column);
        QString getHeaderValueInModel(QAbstractItemModel* model, int section);
        QString prettyPrintMap(QVariantMap map);

        ExecObjects m_exec_obj;
        bool m_command_found;
        bool m_no_error;

        QString m_socket_name;
        QLocalServer *m_server;
        QLocalSocket *m_client_cnx;
};

#endif /* ifdef FUNCTESTS */

#endif /* ifndef __REMOTE_CONTROL_H__ */
