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
#include <QRegExp>

#include <RzxSubnet>

///Construit un sous r�seau � partir d'une ip du r�seau et du masque
/** Cette fonction ne v�rifie pas que les deux adresses sont compatibles.
 * Il faudra faire \ref isValid pour s'assurer que le RzxSubnet est proprement
 * initialis� avec des valeurs coh�rentes.
 */
RzxSubnet::RzxSubnet(const QHostAddress& network, const QHostAddress& netmask)
	:m_network(network), m_netmask(netmask)
{
	normalize();
}

///Construit un sous r�seau � partir d'une adresse et de la taille du masque.
/** Par exemple 129.104.207.0 et 24 donne le sous r�seau 129.104.207.0/255.255.255.0.
 */
RzxSubnet::RzxSubnet(const QHostAddress& network, int mask)
	:m_network(network)
{
	m_netmask = maskFromSize(mask, network.protocol());
	normalize();
}

///Construit une sous r�seau � partir d'une cha�ne l'identifiant
/** La cha�ne doit �tre de la forme ip/mask o� le masque peut �tre soit
 * un entier indiquant la taille du masque, soit une adresse ip.
 *
 * Par exemple, les cha�nes suivantes sont valides :
 * \code
 * RzxSubnet("129.104.207.0/24");
 * RzxSubnet("129.104.207.0/255.255.255.0");
 * \endcode
 */
RzxSubnet::RzxSubnet(const QString& network)
{
	QRegExp mask("(.+)/(.+)");
	if(mask.indexIn(network) == -1) return;

	m_network = QHostAddress(mask.cap(1));
	
	int maskSize;
	bool ok;
	maskSize = mask.cap(2).toInt(&ok, 10);
	if(!ok)
		m_netmask = QHostAddress(mask.cap(2));
	else
		m_netmask = maskFromSize(maskSize, m_network.protocol());
	normalize();
}

///Fonction auxiliaire pour appliquer les masques
QHostAddress operator&(const QHostAddress& network, const QHostAddress& netmask)
{
	if(network.isNull() || netmask.isNull() || network.protocol() != netmask.protocol())
		return QHostAddress();

	switch(network.protocol())
	{
		case QAbstractSocket::IPv4Protocol:
			return QHostAddress(network.toIPv4Address() & netmask.toIPv4Address());
		case QAbstractSocket::IPv6Protocol:
		{
			Q_IPV6ADDR networkValue = network.toIPv6Address();
			Q_IPV6ADDR netmaskValue = netmask.toIPv6Address();
			for(int i = 0 ; i < 16 ; i++)
				networkValue[i] &= netmaskValue[i];
			return QHostAddress(networkValue);
		}
		default:
			return QHostAddress();
	}
}

///Normalise les donn�es pour faciliter l'utilisation
/** En gros on veut que network == network & netmask
 */
void RzxSubnet::normalize()
{
	m_network = m_network & m_netmask;
}

///V�rifie que le sous-r�seau construit est valide
/** Un sous r�seau est valide si :
 * - le network est valide
 * - le netmask est valide
 * - le network et le netmask utilisent le m�me protocole
 */
bool RzxSubnet::isValid() const
{
	if(m_network.isNull()) return false;
	if(m_netmask.isNull()) return false;
	if(m_netmask.protocol() != m_network.protocol()) return false;
	return true;
}

///Retourne le protocole du sous-r�seau
/** Retourne le protocol auquel correspond ce sous-r�seau.
 * Si le sous-r�seau n'est pas valide, cette donn�e n'a pas de sens
 * et sa valeur est alors al�atoire
 */
QAbstractSocket::NetworkLayerProtocol RzxSubnet::protocol() const
{
	return m_network.protocol();
}

///Indique si le sous r�seau contient l'addresse indiqu�e
/** Une adresse appartient � un sous-r�seau si et seulement si
 * network & netmask == addr & netmask.
 *
 * En raison de la normalisation apport�e ici, il nous suffit donc
 * d'avoir network == addr & netmask.
 */
bool RzxSubnet::contains(const QHostAddress &addr) const
{
	return m_network == (addr & m_netmask);
}

///Cr�e un masque adapt� � la taille donn�e
QHostAddress RzxSubnet::maskFromSize(int size, QAbstractSocket::NetworkLayerProtocol protocol)
{
	switch(protocol)
	{
		case QAbstractSocket::IPv4Protocol:
		{
			quint32 mask = 0, bit = 1 << 31;
			for(int i = 0 ; i < size ; i++)
			{
				mask |= bit;
				bit >>= 1;
			}
			return QHostAddress(mask);
		}
		case QAbstractSocket::IPv6Protocol:
		{
			Q_IPV6ADDR mask;
			uchar bit;
			for(int i = 0 ; i < 16 ; i++)
			{
				mask[i] = 0;
				bit = 1 << 7;
				while(size && bit)
				{
					mask[i] |= bit;
					size--;
					bit >>= 1;
				}
			}
			return QHostAddress(mask);
		}
		default:
			return QHostAddress();
	}
}
