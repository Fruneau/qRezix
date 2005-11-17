/***************************************************************************
                          rzxsubnet  -  description
                             -------------------
    begin                : Sat Aug 27 2005
    copyright            : (C) 2005 by Florent Bruneau
    email                : florent.bruneau@m4x.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef RZXSUBNET_H
#define RZXSUBNET_H

#include <QHostAddress>
#include <QAbstractSocket>

#include <RzxGlobal>

/**
 @author Florent Bruneau
 */

///Représentation d'un sous-réseau
/** Un sous-réseau est une plage d'ip définie par une ip et un masque tel que
 * subnet = { ip | ip & masque == ref & masque }.
 */
class RZX_CORE_EXPORT RzxSubnet
{
	QHostAddress m_network;
	QHostAddress m_netmask;

	private:
		void normalize();
		friend QHostAddress operator&(const QHostAddress&, const QHostAddress&);

	public:
		static RzxSubnet unknown;

		RzxSubnet(const QHostAddress&, const QHostAddress&);
		RzxSubnet(const QHostAddress&, int);
		RzxSubnet(const QString&);

		bool isValid() const;
		QAbstractSocket::NetworkLayerProtocol protocol() const;

		const QHostAddress &network() const;
		const QHostAddress &netmask() const;

		bool contains(const QHostAddress&) const;

		static QHostAddress maskFromSize(int, QAbstractSocket::NetworkLayerProtocol);

		QString toString() const;
};

#endif
