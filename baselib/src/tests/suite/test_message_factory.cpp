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

#include <QtTest/QtTest>
#include <QVariantMap>

#include "test_message_factory.h"

#include "message_factory.h"

void TestMessageFactory::testSubscribeCurrentCalls()
{
    QVariantMap result = MessageFactory::subscribeCurrentCalls();

    QVariantMap expected;
    expected["class"] = "subscribe";
    expected["message"] = "current_calls";

    QCOMPARE(result, expected);
}

void TestMessageFactory::testAnswer()
{
    QVariantMap result = MessageFactory::answer();

    QVariantMap expected;
    expected["class"] = "answer";

    QCOMPARE(result, expected);
}

void TestMessageFactory::testHangup()
{
    QVariantMap result = MessageFactory::hangup();

    QVariantMap expected;
    expected["class"] = "hangup";

    QCOMPARE(result, expected);
}

void TestMessageFactory::testHoldSwitchboard()
{
    QVariantMap result = MessageFactory::holdSwitchboard();

    QVariantMap expected;
    expected["class"] = "hold_switchboard";

    QCOMPARE(result, expected);
}

void TestMessageFactory::testUnholdSwitchboard()
{
    QString unique_id = "1287634.33";
    QVariantMap result = MessageFactory::unholdSwitchboard(unique_id);

    QVariantMap expected;
    expected["class"] = "unhold_switchboard";
    expected["unique_id"] = unique_id;

    QCOMPARE(result, expected);
}

void TestMessageFactory::testAttendedTransfer()
{
    QString number = "1234";
    QVariantMap result = MessageFactory::attendedTransfer(number);

    QVariantMap expected;
    expected["class"] = "attended_transfer";
    expected["number"] = number;

    QCOMPARE(result, expected);
}