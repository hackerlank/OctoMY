#include "KeyStore.hpp"
#include "utility/Utility.hpp"
#include <QJsonObject>
#include <QJsonParseError>
#include <QFile>

QString fn="keystore.json";
quint32 keyBits=4096;

KeyStore::KeyStore(){
	bootstrap();
}



KeyStore::~KeyStore(){
	save();
}


void KeyStore::bootstrap(){
	QFile f(fn);
	if(!f.exists()){
		qDebug()<<"KEYSTORE: no keystore file found, generating local keypair and saving";
		local_pki.reset();
		local_pki.generateKeyPair(keyBits);
		save();
	}
	load();
}


void KeyStore::load(){
	qDebug()<<"KEYSTORE: Loading from file";
	QJsonParseError jsonError;
	QJsonDocument doc = QJsonDocument::fromJson(utility::fileToByteArray(fn), &jsonError);
	if (jsonError.error != 0){
		qWarning() << "ERROR: Parsing json data: "<<jsonError.errorString();
	}
	else{
		//qDebug()<<"PARSED JSON: "<<doc.toJson();
		QVariantMap map = doc.object().toVariantMap();
		local_pki.reset();
		local_pki.parseKey(map["localPrivateKey"].toByteArray());
		local_pki.parsePublicKey(map["localPublicKey"].toByteArray());
		QVariantList remotes=map["remotePublicKeys"].toList();
		peer_pki.clear();
		for(QVariantList::iterator b=remotes.begin(), e=remotes.end(); b!=e; ++b){
			QVariantMap remote=(*b).toMap();
			peer_pki[remote["id"].toString()]->parsePublicKey(remote["PublicKey"].toByteArray());
		}
	}
}

void KeyStore::save(){
	qDebug()<<"KEYSTORE: Saving to file";
	QVariantMap map;
	map["createdTimeStamp"]=QDateTime::currentMSecsSinceEpoch();
	map["localPrivateKey"]=local_pki.getPEMKey();
	map["localPublicKey"]=local_pki.getPEMPubkey();
	QVariantList remotes;
	for(QMap<QString, QSharedPointer<qpolarssl::Pki> >::iterator b=peer_pki.begin(), e=peer_pki.end(); b!=e; ++b){
		QVariantMap remote;
		remote["id"]=b.key();
		remote["PublicKey"]=b.value()->getPEMPubkey();
		remotes.push_back(remote);
	}
	map["remotePublicKeys"]=remotes;
	QJsonDocument doc=QJsonDocument::fromVariant(map);
	utility::stringToFile(fn,doc.toJson());
}

QByteArray KeyStore::sign(const QByteArray &source){
	return local_pki.sign(source,qpolarssl::THash::SHA256);
}


bool KeyStore::verify(const QByteArray &message, const QByteArray &signature){
	return local_pki.verify(message, signature,qpolarssl::THash::SHA256);
}


bool KeyStore::verify(const QString &fingerprint, const QByteArray &message, const QByteArray &signature){
	QMap<QString, QSharedPointer<qpolarssl::Pki> >::iterator f=peer_pki.find(fingerprint);
	if(peer_pki.end()==f){
		return false;
	}
	QSharedPointer<qpolarssl::Pki> remote_pki=f.value();
	if(remote_pki){
		return remote_pki->verify(message, signature,qpolarssl::THash::SHA256);
	}
	return false;
}


bool KeyStore::hasPubKeyForFingerprint(const QString &fingerprint){
	return (peer_pki.end()==peer_pki.find(fingerprint));
}

void KeyStore::setPubKeyForFingerprint(const QString &fingerprint,const QString &pubkeyPEM){
	QSharedPointer<qpolarssl::Pki> remote_pki=QSharedPointer<qpolarssl::Pki>(new qpolarssl::Pki);
	remote_pki->parsePublicKey(pubkeyPEM.toUtf8());
	peer_pki[fingerprint]=remote_pki;
}


QString KeyStore::getLocalPublicKey(){
	return local_pki.getPEMPubkey();
}

QString KeyStore::getLocalPrivateKey(){
	return local_pki.getPEMKey();
}

/*

void KeyStore::verifyKeypair(){
	//const auto priPath = QString("/home/lennart/keypairs/private_key.pem"); 	const auto pubPath = QString("/home/lennart/keypairs/public_key.pem"); utility::fileToByteArray(pubPath);
	int ret=0;
	qpolarssl::Pki pkiPrivate;
	QByteArray priKeyData  = ui->plainTextEditPrivateKey->toPlainText().toUtf8();
	if( (ret=pkiPrivate.parseKey(priKeyData) != 0) ){
		QMessageBox msgBox;
		msgBox.setIcon(QMessageBox::Critical);
		msgBox.setText("Parsing of private key failed with code "+QString::number(ret));
		msgBox.exec();
		return;
	}
	else if(!pkiPrivate.isValid()){
		QMessageBox msgBox;
		msgBox.setIcon(QMessageBox::Critical);
		msgBox.setText("PKI was invalid after parsing of private key");
		msgBox.exec();
		return;
	}

	qpolarssl::Pki pkiPublic;
	QByteArray pubKeyData  = ui->plainTextEditPublicKey->toPlainText().toUtf8();
	if( (ret=pkiPublic.parsePublicKey(pubKeyData) != 0) ){
		QMessageBox msgBox;
		msgBox.setIcon(QMessageBox::Critical);
		msgBox.setText("Parsing of public key failed with code -"+QString::number(-ret,16));
		msgBox.exec();
		return;
	}
	else if(!pkiPublic.isValid()){
		QMessageBox msgBox;
		msgBox.setIcon(QMessageBox::Critical);
		msgBox.setText("PKI was invalid after parsing of public key");
		msgBox.exec();
		return;
	}

	const QByteArray sourceData = QString("OctoMY™ test data in cleartext").toUtf8();

	const QByteArray signature = pkiPrivate.sign(sourceData, qpolarssl::THash::SHA256);

	const int siglen=signature.length();
	if((siglen <= 64) ){
		QMessageBox msgBox;
		msgBox.setIcon(QMessageBox::Critical);
		msgBox.setText("Signature size "+QString::number(siglen)+" was less than 64");
		msgBox.exec();
		return;
	}

	ret = pkiPublic.verify(sourceData, signature, qpolarssl::THash::SHA256);

	if(ret!=0){
		QMessageBox msgBox;
		msgBox.setIcon(QMessageBox::Critical);
		msgBox.setText("Signature verification failed with code -"+QString::number(-ret,16));
		msgBox.exec();
		return;
	}

	QMessageBox msgBox;
	msgBox.setIcon(QMessageBox::Information);
	msgBox.setText("Signature verification succeeded.");
	msgBox.exec();
}
*/