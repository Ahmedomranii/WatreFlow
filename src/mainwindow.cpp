#include "mainwindow.h"
#include "personnel.h"
#include "livraisons.h"

#include <QApplication>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QStackedWidget>
#include <QStyle>
#include <QVBoxLayout>

namespace {
// Ordre = ordre du menu = ordre des pages dans le QStackedWidget.
enum Section {
    TableauDeBord, Personnel_, Equipements, ProductionStocks,
    Commandes, Livraisons_, Statistiques, Alertes
};
}

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    m_rubriques = {"Tableau de bord", "Personnel", "Équipements", "Production & stocks",
                   "Commandes", "Livraisons", "Statistiques", "Alertes"};

    auto *central = new QWidget(this);
    auto *principal = new QHBoxLayout(central);
    principal->setContentsMargins(0, 0, 0, 0);
    principal->setSpacing(0);

    // ---- Barre latérale ----
    auto *sidebar = new QFrame(central);
    sidebar->setObjectName("sidebar");
    sidebar->setFixedWidth(190);
    auto *layoutSidebar = new QVBoxLayout(sidebar);
    layoutSidebar->setContentsMargins(12, 12, 12, 12);
    layoutSidebar->setSpacing(14);

    auto *logo = new QLabel("◈  WaterFlow", sidebar);
    logo->setObjectName("labelLogo");
    layoutSidebar->addWidget(logo);

    m_menu = new QListWidget(sidebar);
    m_menu->setObjectName("menuLateral");
    m_menu->setFocusPolicy(Qt::NoFocus);
    m_menu->addItems(m_rubriques);

    const QList<QStyle::StandardPixmap> icones = {
        QStyle::SP_ComputerIcon, QStyle::SP_DirHomeIcon, QStyle::SP_DriveHDIcon,
        QStyle::SP_FileDialogDetailedView, QStyle::SP_ArrowRight, QStyle::SP_DialogOpenButton,
        QStyle::SP_FileDialogInfoView, QStyle::SP_MessageBoxWarning
    };
    for (int i = 0; i < m_menu->count() && i < icones.size(); ++i) {
        m_menu->item(i)->setIcon(QApplication::style()->standardIcon(icones[i]));
        m_menu->item(i)->setSizeHint(QSize(0, 44));
    }
    layoutSidebar->addWidget(m_menu, 1);

    // ---- Pages ----
    m_pages = new QStackedWidget(central);
    for (int i = 0; i < m_rubriques.size(); ++i) {
        switch (i) {
        case Personnel_:  m_pages->addWidget(new Personnel(m_pages));   break;
        case Livraisons_: m_pages->addWidget(new Livraisons(m_pages));  break;
        default:          m_pages->addWidget(creerPlaceholder(m_rubriques[i])); break;
        }
    }

    principal->addWidget(sidebar);
    principal->addWidget(m_pages, 1);
    setCentralWidget(central);

    connect(m_menu, &QListWidget::currentRowChanged, this, &MainWindow::afficherSection);
    m_menu->setCurrentRow(Personnel_);   // page d'accueil après connexion
}

void MainWindow::afficherSection(int row)
{
    if (row < 0 || row >= m_pages->count())
        return;
    m_pages->setCurrentIndex(row);
    setWindowTitle("WaterFlow | " + m_rubriques.value(row));
}

QWidget *MainWindow::creerPlaceholder(const QString &titre)
{
    auto *page = new QWidget;
    auto *l = new QVBoxLayout(page);
    l->setContentsMargins(16, 16, 16, 16);
    l->setSpacing(6);

    auto *t = new QLabel(titre, page);
    t->setObjectName("labelTitre");
    auto *s = new QLabel("Cette section sera bientôt disponible.", page);
    s->setObjectName("placeholderSub");
    l->addWidget(t);
    l->addWidget(s);
    l->addStretch(1);
    return page;
}
