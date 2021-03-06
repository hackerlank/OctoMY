#include "MultiView.hpp"
#include "ui_MultiView.h"
#include "basic/Settings.hpp"
#include "../libutil/utility/Standard.hpp"

#include <QDebug>

// Inspired by http://doc.qt.io/qt-5/qtwidgets-itemviews-interview-example.html

MultiView::MultiView(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::MultiView)
{
	ui->setupUi(this);
	QListView *list = ui->listView;
	if(0!=list){
		list->setViewMode(QListView::IconMode);
		list->setSelectionMode(QAbstractItemView::ExtendedSelection);
		list->setAlternatingRowColors(false);
		list->viewport()->setAttribute(Qt::WA_StaticContents);
		list->setAttribute(Qt::WA_MacShowFocusRect, false);
	}
	QTableView *table = ui->tableView;
	if(0!=table ){
		table->horizontalHeader()->setSectionsMovable(true);
		table->verticalHeader()->setSectionsMovable(true);
		// Set StaticContents to enable minimal repaints on resizes.
		table->viewport()->setAttribute(Qt::WA_StaticContents);
	}
	QTreeView *tree = ui->treeView;
	if(0!=tree ){
		tree->setUniformRowHeights(true);
		tree->header()->setStretchLastSection(false);
		tree->viewport()->setAttribute(Qt::WA_StaticContents);
		// Disable the focus rect to get minimal repaints when scrolling on Mac.
		tree->setAttribute(Qt::WA_MacShowFocusRect, false);
	}
	//Start of with no data
	setModel(nullptr);
	if(!connect(ui->buttonGroupView,SIGNAL(buttonClicked(QAbstractButton*)),this,SLOT(onViewButtonClicked(QAbstractButton*)),OC_CONTYPE)){
		qDebug()<<"ERROR: could not connect";
	}
	//Trigger change
	ui->pushButtonViewList->click();
}

void MultiView::setModel(QAbstractItemModel *data){
	QItemSelectionModel *selections = 0==data?0:new QItemSelectionModel(data);
	QListView *list = ui->listView;
	if(nullptr!=list){
		QItemSelectionModel *oldSel = list->selectionModel();
		list->setModel(data);
		delete oldSel;
		if(nullptr!=selections){
			list->setSelectionModel(selections);
		}
	}
	QTableView *table = ui->tableView;
	if(nullptr!=table ){
		QItemSelectionModel *oldSel = table->selectionModel();
		table->setModel(data);
		delete oldSel;
		oldSel=nullptr;
		if(nullptr!=selections){
			table->setSelectionModel(selections);
		}
	}
	QTreeView *tree = ui->treeView;
	if(nullptr!=tree ){
		QItemSelectionModel *oldSel = tree->selectionModel();
		tree->setModel(data);
		delete oldSel;
		oldSel=nullptr;
		if(nullptr!=selections){
			tree->setSelectionModel(selections);
		}
	}
}

MultiView::~MultiView(){
	delete ui;
}


void MultiView::onViewButtonClicked(QAbstractButton*but){
	QAbstractButton*c=ui->buttonGroupView->checkedButton();
	QString item="";
	if(c!=but && nullptr != but){
		qDebug()<<"CLICKING BUTTON: "<<but->objectName();
		but->click();
	}
	if(but==ui->pushButtonViewGrid){
		ui->stackedWidget->setCurrentWidget(ui->pageList);
		//ui->listView->clearPropertyFlags();
		ui->listView->setViewMode(QListView::IconMode);
		item="Grid";
	}
	else if(but==ui->pushButtonViewList){
		ui->stackedWidget->setCurrentWidget(ui->pageList);
		//ui->listView->clearPropertyFlags();
		ui->listView->setViewMode(QListView::ListMode);
		item="List";
	}
	else if(but==ui->pushButtonViewDetails){
		ui->stackedWidget->setCurrentWidget(ui->pageTable);
		item="Details";
	}
	else if(but==ui->pushButtonViewTree){
		ui->stackedWidget->setCurrentWidget(ui->pageTree);
		item="Tree";
	}
	else{
		qDebug()<<"ERROR: Unknwon button click detected";
		return;
	}
	if(nullptr!=settings){
		settings->setCustomSetting(k,item);
	}
}



void MultiView::configure(Settings &s, QString val, QString key){
	settings=&s;
	k=key.trimmed();
	if(""!=k && nullptr!=settings){
		val=settings->getCustomSetting(k,val);
	}
	QAbstractButton *but=nullptr;
	if("Grid"==val){
		but=ui->pushButtonViewGrid;
	}
	else if("List"==val){
		but=ui->pushButtonViewList;
	}
	else if("Details"==val){
		but=ui->pushButtonViewDetails;
	}
	else if("Tree"==val){
		but=ui->pushButtonViewTree;
	}
	else{
		qDebug()<<"ERROR: NO SUITABLE BUTTON NAME FOUDND FOR "<<val;
		return;
	}
	//onViewButtonClicked(but);
}

