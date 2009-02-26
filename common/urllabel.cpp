/* XIVO CTI clients
 * Copyright (C) 2007-2009  Proformatique
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License version 2 for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Linking the Licensed Program statically or dynamically with other
 * modules is making a combined work based on the Licensed Program. Thus,
 * the terms and conditions of the GNU General Public License version 2
 * cover the whole combination.
 *
 * In addition, as a special exception, the copyright holders of the
 * Licensed Program give you permission to combine the Licensed Program
 * with free software programs or libraries that are released under the
 * GNU Library General Public License version 2.0 or GNU Lesser General
 * Public License version 2.1 or any later version of the GNU Lesser
 * General Public License, and with code included in the standard release
 * of OpenSSL under a version of the OpenSSL license (with original SSLeay
 * license) which is identical to the one that was published in year 2003,
 * or modified versions of such code, with unchanged license. You may copy
 * and distribute such a system following the terms of the GNU GPL
 * version 2 for the Licensed Program and the licenses of the other code
 * concerned, provided that you include the source code of that other code
 * when and as the GNU GPL version 2 requires distribution of source code.
*/

/* $Revision$
 * $Date$
 */

#include <QLabel>

#include "urllabel.h"

/*! \brief Constructor
 *
 * Call QLabel constructor, set properties to
 * allow link to be clickable and put some html magic
 * into the label text. */
UrlLabel::UrlLabel(const QString & url, QWidget * parent)
        : QLabel(parent)
{
        QString displaytext, text;
        //setForegroundRole( QPalette::Link );
        setOpenExternalLinks(true);        // new in Qt 4.2
        setTextInteractionFlags( Qt::LinksAccessibleByMouse
                                 | Qt::LinksAccessibleByKeyboard );
        //                        | Qt::TextSelectableByMouse
        //                        | Qt::TextSelectableByKeyboard );
        if(url.startsWith("mailto:"))
                displaytext = url.mid(7);
        else if(url.startsWith("dial:"))
                displaytext = url.mid(5);
        else
                displaytext = url;
        //qDebug() << "UrlLabel::UrlLabel()" << url << displaytext;
        text = "<a href=\"";
        text.append(url);
        text.append("\">");
        text.append(displaytext);
        text.append("</a>");
        setText(text);
}

