#ifndef PAIRINGWIZARD_HPP
#define PAIRINGWIZARD_HPP

#include "comms/discovery/DiscoveryClient.hpp"

#include <QWidget>



namespace Ui {
	class PairingWizard;
}

class Node;
class PairingTable;
class PairingList;

class EditButtonDelegate;

class PairingWizard : public QWidget
{
		Q_OBJECT

	private:
		Ui::PairingWizard *ui;
		Node *node;
		PairingList *list;
		EditButtonDelegate *delegate;

	public:
		explicit PairingWizard(QWidget *parent = 0);
		virtual ~PairingWizard();
	public:

		void configure(Node *node);

	public:
		void reset();

		void startEdit(int row);

		Node *getNode();

	protected:

		virtual void showEvent(QShowEvent *);
		virtual void hideEvent(QHideEvent *);

	private slots:
		void on_pushButtonMaybeOnward_clicked();
		void on_pushButtonTryAgain_clicked();
		void on_pushButtonDone_clicked();
		void on_pushButtonCameraPair_clicked();

		void on_pushButtonDiscoverNow_clicked();

		void on_pushButtonSaveEdits_clicked();

	signals:
		void done();
};

#endif // PAIRINGWIZARD_HPP