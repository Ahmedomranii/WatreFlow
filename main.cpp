#include <QApplication>
#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QSettings>
#include <QCryptographicHash>
#include <QFileInfo>
#include "personnel.h"

static const char *STYLE = R"(
QWidget { background:#E6F1F8; color:#1b2a38; font-family:"Segoe UI"; font-size:9pt; }
QLabel { background:transparent; }
QFrame#sidebar { background:#0A4D8C; }
QLabel#labelLogo { color:white; font-size:15pt; font-weight:bold; padding:6px 4px 10px 4px; }
QListWidget#menuLateral { background:transparent; color:white; border:none; font-size:10pt; outline:none; }
QListWidget#menuLateral::item { padding:9px 12px; border-radius:6px; margin:1px 0; }
QListWidget#menuLateral::item:selected { background:#00B4D8; color:white; }
QLabel#labelTitre { color:#0A4D8C; font-size:16pt; font-weight:bold; }
QFrame#frameAlerte { background:white; border:1px solid #F3E3BF; border-left:4px solid #F39C12; border-radius:6px; }
QFrame#cardListe, QFrame#cardFiche, QFrame#cardPlanning, QFrame#cardGraphique { background:white; border:1px solid #D6E6EF; border-radius:8px; }
QLabel#labelTitreListe, QLabel#labelTitreFiche, QLabel#labelTitrePlanning, QLabel#labelTitreGraphique { color:#0A4D8C; font-weight:bold; font-size:9pt; }
QPushButton { background:#00B4D8; color:white; border:none; border-radius:5px; padding:7px 14px; font-weight:bold; }
QPushButton:hover { background:#0A4D8C; }
QPushButton#btnSupprimer { background:#E74C3C; }
QLineEdit, QComboBox, QDateEdit { background:white; border:1px solid #CFE0EA; border-radius:4px; padding:5px 6px; }
QTableWidget { background:white; border:none; alternate-background-color:#F4F9FC; selection-background-color:#DDF3FA; selection-color:#1b2a38; }
QTableWidget::item { padding-left:4px; border-bottom:1px solid #EAF1F6; }
QHeaderView::section { background:#0A4D8C; color:white; padding:6px; border:none; font-weight:bold; }
QDialog#loginDialog { background:#E6F1F8; }
QLabel#loginTitle { color:#0A4D8C; font-size:20pt; font-weight:bold; }
QLabel#loginSub { color:#5B6B7B; }
QLineEdit#loginUser, QLineEdit#loginPass { padding:10px; font-size:10pt; }
QPushButton#loginButton { padding:10px; font-size:10pt; }
)";

static QString hashPassword(const QString &s) {
    return QString(QCryptographicHash::hash(s.toUtf8(), QCryptographicHash::Sha256).toHex());
}

class LoginDialog : public QDialog {
public:
    explicit LoginDialog(QWidget *parent=nullptr) : QDialog(parent) {
        setObjectName("loginDialog"); setWindowTitle("WaterFlow - Connexion"); setFixedSize(390, 330);
        auto *layout=new QVBoxLayout(this); layout->setContentsMargins(42,32,42,32); layout->setSpacing(12);
        auto *title=new QLabel("WaterFlow"); title->setObjectName("loginTitle"); title->setAlignment(Qt::AlignCenter);
        auto *sub=new QLabel("Connexion utilisateur"); sub->setObjectName("loginSub"); sub->setAlignment(Qt::AlignCenter);
        user=new QLineEdit; user->setObjectName("loginUser"); user->setPlaceholderText("Nom d'utilisateur");
        pass=new QLineEdit; pass->setObjectName("loginPass"); pass->setPlaceholderText("Mot de passe"); pass->setEchoMode(QLineEdit::Password);
        auto *btn=new QPushButton("Se connecter"); btn->setObjectName("loginButton");
        error=new QLabel; error->setStyleSheet("color:#E74C3C;"); error->setAlignment(Qt::AlignCenter); error->setWordWrap(true);
        layout->addWidget(title); layout->addWidget(sub); layout->addSpacing(8); layout->addWidget(user); layout->addWidget(pass); layout->addWidget(btn); layout->addWidget(error);
        connect(btn,&QPushButton::clicked,this,&LoginDialog::login); connect(pass,&QLineEdit::returnPressed,this,&LoginDialog::login);
        QSettings s("WaterFlow","Personnel");
        if (!s.contains("auth/admin")) { s.setValue("auth/admin", hashPassword("admin123")); }
    }
private:
    void login() {
        QSettings s("WaterFlow","Personnel");
        const QString u=user->text().trimmed(); const QString p=pass->text();
        if (u.isEmpty() || p.isEmpty()) { error->setText("Veuillez remplir les deux champs."); return; }
        if (s.value("auth/"+u).toString() == hashPassword(p)) accept();
        else error->setText("Identifiant ou mot de passe incorrect.");
    }
    QLineEdit *user{}, *pass{}; QLabel *error{};
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv); app.setStyleSheet(STYLE);
    LoginDialog login;
    if (login.exec()!=QDialog::Accepted) return 0;
    Personnel fenetre; fenetre.setWindowTitle("WaterFlow - Gestion du personnel"); fenetre.resize(1240,780); fenetre.show();
    return app.exec();
}
