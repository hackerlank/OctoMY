#ifndef COMMSCHANNEL_HPP
#define COMMSCHANNEL_HPP

#include <QObject>
#include <QHostAddress>
#include <QUdpSocket>
#include <QList>
#include <QMap>
#include <QTimer>
#include <QDateTime>

#include "ReliabilitySystem.hpp"
#include "FlowControl.hpp"

#include "basic/LogDestination.hpp"
#include "couriers/Courier.hpp"

#define TIMEOUT_TRESHOLD 1

#define OCTOMY_PROTOCOL_MAGIC (0x0C701111)
#define OCTOMY_PROTOCOL_VERSION_CURRENT (1)
#define OCTOMY_PROTOCOL_DATASTREAM_VERSION_CURRENT (QDataStream::Qt_5_5)



class HubWindow;
class Client;
class ClientDirectory;
class ClientSignature;

/*
Architecture:

TCP tries to pretend on behalf of the user that network traffic is a dependable
linear unbroken stream of bytes. From the user's perspective this has the
benefit of being easy to understand and use. The two major downsides however
are that:

1. This far from how a network actually works and so it is hard to make TCP work
   it has taken decades of evolution for TCP to become as good as it is today,
   but it is still limited by this fallacy

2. Some needs may actually be better met by not thinking about the network as a
   linear stream of bytes, so going the length to pretend that it is actually
   just gets in the way.

CommChannel is an API wrapping UDP in Qt in a way that allows the user to
exploit the benefits of communications over UDP by modelling closesly the
benefits in a way that hides their inherent complexities.

The CommChannel API works like this:

1. Users of the API register "couriers" that each is responsible for keeping the
   latest data fresh and ready for sending should an opportunity present itself.
   Couriers each tend to a certain type of packet with a certain priority and
   desired sending frequency. It is up to each courier to maintain it's own
   state.

2. CommChannel is in charge and decides the speed at which packets are sent and
   which couriers get their packets sent on each opportunity.

3. CommChannel may at any time send non-payload data in each packet or even
   special purpose network-only packets to sustain its operation. If there is no
   data to be sent by couriers, CommChannel may send no-op packets that
   facilitates the calculation of the network characteristics such as
   round trip time. This is done transparentyl to the couriers.


Notes:

1. CommChannel binds to a local address and port, but does not really
   discriminate from where inbound traffic arrives. All packets are treated
   equal as all packes contain identification of the source.

2. Communication between remote and agent is initiated when user presses the "connect"
   buttons in the respective user interfaces.

3.  At that point agent will attemt to contact all trusted remotes at their last
   known address untill one or more answer are received. From then on, all
   connections that did not result in valid responses will be closed, and only
   retried periodically.

4. All Agent initiadet communication will  be broadcast to all active remotes in
   paralellesin. Remote initiated transfers will remain private.

*/
class CommsChannel : public QObject
{
	Q_OBJECT
private:
	static const quint64 CONNECTION_TIMEOUT=1000*5;//5 sec connection timeout
	QUdpSocket mUDPSocket;
	QTimer mSendingTimer;
	//Clients as identifued by their fingerprints
	ClientDirectory *mClients;
	LogDestination *mLog;
	//All registered couriers
	QList<Courier *> mCouriers;
	QMap<quint32, Courier *> mCouriersByID;
	ClientSignature mLocalSignature;
	quint64 mLastRX;
	quint64 mLastTX;
	quint64 mLastRXST;
	quint64 mLastTXST;
	quint64 mTxCount;
	quint64 mRxCount;
	bool mConnected;

	//Receive counter used in debug messages to make sense of the order of things
	static quint32 totalRecCount;

public:

	explicit CommsChannel(const QString &id, LogDestination *mLog=nullptr, QObject *parent=nullptr);

public:

	ClientDirectory *clients();

	void start(NetworkAddress localAddress);
	void stop();

	ClientSignature localSignature();

	void setLogOutput(LogDestination *log);

	QString getSummary();

	void setID(const QString &id);

	void hookSignals(QObject &ob);
	void unHookSignals(QObject &ob);

	void registerCourier(Courier &);
	void unregisterCourier(Courier &);
	int courierCount();

	inline bool hasCourier(Courier &c)
	{
		return mCouriers.contains(&c);
	}

	inline Courier *getCourierByID(quint32 id)
	{
		if(mCouriersByID.contains(id)) {
			return mCouriersByID[id];
		}
		return nullptr;
	}

	bool isStarted()
	{
		return mSendingTimer.isActive();
	}
	bool isConnected()
	{
		return mConnected;
	}

	qint64 sendRawData(QByteArray datagram,ClientSignature sig);


private:

	void appendLog(QString);

	void sendData(const quint64 &now, QSharedPointer<Client> localClient, Courier *courier, const ClientSignature *sig=nullptr);

	// CommsChannel signals
signals:
	//		void receivePacket(QSharedPointer<QDataStream> data,QHostAddress host, quint16 port);
	void commsError(QString message);
	void commsClientAdded(Client *c);
	void commsConnectionStatusChanged(bool c);

private slots:

	void receivePacketRaw(QByteArray ba,QHostAddress host, quint16 port);
	void onSendingTimer();

public slots:

	void onReadyRead();
	void onUdpError(QAbstractSocket::SocketError);
};

#endif // COMMSCHANNEL_HPP
