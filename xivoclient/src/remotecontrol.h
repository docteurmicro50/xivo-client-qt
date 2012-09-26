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

#ifndef __REMOTECONTROL_H__
#define __REMOTECONTROL_H__

#ifdef FUNCTESTS

#include <QObject>
#include <string>
#include <exception>

using namespace std;

#include "main.h"

class QLocalServer;
class QLocalSocket;

struct RemoteControlCommand {
    QString action;
    QStringList arguments;
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
        void i_log_in_the_xivo_client_to_host_1_as_2_pass_3(const QStringList &);
        void i_log_in_the_xivo_client_to_host_1_as_2_pass_3_unlogged_agent(const QStringList &);
        void i_log_out_of_the_xivo_client();
        void then_the_xlet_identity_shows_name_as_1_2(const QStringList &);
        void then_the_xlet_identity_shows_server_name_as_field_1_modified(const QStringList &);
        void then_the_xlet_identity_shows_phone_number_as_1(const QStringList &);
        void then_the_xlet_identity_shows_a_voicemail_1(const QStringList &);
        void then_the_xlet_identity_shows_an_agent_1(const QStringList &);
        void then_the_xlet_identity_does_not_show_any_agent();
        void when_i_enable_screen_pop_up();
        void then_i_see_a_sheet_with_variables_and_values(const QStringList &);

    public slots:
        void on_error(const QString &);

    private slots:
        void newConnection();
        void processCommands();

    private:
        RemoteControlCommand parseCommand(const QByteArray & raw_command);
        void sendResponse(RemoteControlResponse response, const char * message = "");
        void assert(bool condition, const QString & message = "");
        bool commandMatches(RemoteControlCommand, std::string);

        ExecObjects m_exec_obj;
        bool m_command_found;
        bool m_no_error;

        QString m_socket_name;
        QLocalServer *m_server;
        QLocalSocket *m_client_cnx;
};

#endif /* ifdef FUNCTESTS */

#endif /* ifndef __REMOTECONTROL_H__ */
