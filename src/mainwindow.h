#pragma once
#include <QMainWindow>
#include <QStringList>

class QListWidget;
class QStackedWidget;

// Fenêtre principale : barre latérale commune + une page par module.
class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);

private slots:
    void afficherSection(int row);

private:
    QWidget *creerPlaceholder(const QString &titre);

    QListWidget *m_menu = nullptr;
    QStackedWidget *m_pages = nullptr;
    QStringList m_rubriques;
};
