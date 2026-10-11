#include "livraisons.h"
#include "ui_livraisons.h"
#include <QStandardItemModel>
#include <QColor>
#include <QApplication>
#include <QStyle>
#include <QHeaderView>
#include <QFont>

Livraisons::Livraisons(QWidget *parent) : QWidget(parent), ui(new Ui::Livraisons)
{
    ui->setupUi(this);

    // Icônes natives Qt : aucun fichier d’icône externe n’est nécessaire.
    ui->btnRechercher->setIcon(style()->standardIcon(QStyle::SP_FileDialogContentsView));
    ui->btnAjouter->setIcon(style()->standardIcon(QStyle::SP_FileDialogNewFolder));
    ui->btnModifier->setIcon(style()->standardIcon(QStyle::SP_FileDialogDetailedView));
    ui->btnSupprimer->setIcon(style()->standardIcon(QStyle::SP_TrashIcon));
    ui->btnPdf->setIcon(style()->standardIcon(QStyle::SP_DialogSaveButton));
    ui->tableView->setShowGrid(false);
    ui->tableView->setAlternatingRowColors(true);
    ui->tableView->verticalHeader()->setDefaultSectionSize(38);
    ui->tableView->horizontalHeader()->setMinimumSectionSize(90);
    ui->tableView->horizontalHeader()->setStretchLastSection(true);

    // Données d'exemple (juste pour l'affichage)
    auto *model = new QStandardItemModel(this);
    model->setHorizontalHeaderLabels({"ID", "Commande", "Chauffeur", "Zone", "Départ", "Statut"});

    const QList<QStringList> rows = {
        {"501", "101", "Ali Ben Salah",  "Tunis Nord", "02/10 08:00", "En cours"},
        {"502", "102", "Sami Trabelsi",  "Ariana",     "02/10 09:30", "Planifiée"},
        {"503", "103", "Ali Ben Salah",  "Ben Arous",  "30/09 07:00", "Livrée"},
        {"504", "104", "Nizar Gharbi",   "La Marsa",   "01/10 10:00", "Retardée"},
    };
    const QColor couleurs[] = {QColor("#2878F0"), QColor("#D97706"), QColor("#16845B"), QColor("#C03945")};

    for (int r = 0; r < rows.size(); ++r) {
        QList<QStandardItem *> ligne;
        for (const QString &texte : rows[r])
            ligne << new QStandardItem(texte);
        ligne.last()->setForeground(couleurs[r]);   // couleur du statut
        ligne.last()->setFont(QFont("Segoe UI", 9, QFont::DemiBold));
        model->appendRow(ligne);
    }
    ui->tableView->setModel(model);
}

Livraisons::~Livraisons() { delete ui; }
