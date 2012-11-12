/* XiVO Client
 * Copyright (C) 2007-2012, Avencall
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

#include <baseengine.h>
#include "agents_model.h"
#include "agents_view.h"

AgentsView::AgentsView(QWidget *parent)
    : AbstractTableView(parent)
{
    connect(this, SIGNAL(clicked(const QModelIndex &)),
            this, SLOT(agentClicked(const QModelIndex &)));
}

void AgentsView::agentClicked(const QModelIndex &index)
{
    QModelIndex agent_id_index = model()->index(index.row(),
                                                AgentsModel::ID,
                                                index.parent());
    QString agent_id = model()->data(agent_id_index).toString();

    int column = index.column();
    switch(column) {
    case AgentsModel::LISTEN:
        this->listenAgent(agent_id);
        break;
    case AgentsModel::LOGGED_STATUS:
        this->logAgent(agent_id);
        break;
    case AgentsModel::PAUSED_STATUS:
        this->pauseAgent(agent_id);
        break;
    default:
        changeWatchedAgent(agent_id);
    }
}

void AgentsView::changeWatchedAgent(const QString & agent_id)
{
    b_engine->changeWatchedAgent(agent_id, false);
}

void AgentsView::listenAgent(const QString & agent_id)
{
    QVariantMap ipbxcommand;
    ipbxcommand["command"] = "listen";
    ipbxcommand["subcommand"] =  "start";
    ipbxcommand["destination"] = agent_id;
    b_engine->ipbxCommand(ipbxcommand);
}

void AgentsView::logAgent(const QString & agent_id)
{
    const AgentInfo * agent = b_engine->agent(agent_id);
    if (agent == NULL) {
        return;
    }
    QVariantMap ipbxcommand;
    ipbxcommand["agentids"] = agent_id;
    if (agent->logged()) {
        ipbxcommand["command"] = "agentlogout";
    } else {
        ipbxcommand["command"] = "agentlogin";
    }
    b_engine->ipbxCommand(ipbxcommand);
}

void AgentsView::pauseAgent(const QString & agent_id)
{
    const AgentInfo * agent = b_engine->agent(agent_id);
    if (agent == NULL) {
        return;
    }
    QVariantMap ipbxcommand;
    enum AgentPauseStatus pause_status = agent->pausedStatus();
    switch (pause_status) {
    case UNPAUSED:
    case PARTIALLY_PAUSED:
        ipbxcommand["command"] = "queuepause";
        break;
    case PAUSED:
        ipbxcommand["command"] = "queueunpause";
        break;
    }
    ipbxcommand["member"] = QString("agent:%0").arg(agent_id);
    ipbxcommand["queue"] = QString("queue:xivo/all");

    b_engine->ipbxCommand(ipbxcommand);
}