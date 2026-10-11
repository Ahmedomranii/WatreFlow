#include <QApplication>
#include "logindialog.h"
#include "mainwindow.h"
#include "style.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    // Même nom d'application/organisation que le module Personnel d'origine :
    // le fichier employes.json existant est donc retrouvé tel quel.
    app.setApplicationName("WaterFlow");
    app.setOrganizationName("WaterFlow");
    app.setStyle("Fusion");
    app.setStyleSheet(kStyleSheet);

    LoginDialog login;
    if (login.exec() != QDialog::Accepted)
        return 0;

    MainWindow fenetre;
    fenetre.resize(1440, 900);
    fenetre.show();
    return app.exec();
}
