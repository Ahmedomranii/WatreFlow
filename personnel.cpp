#include "personnel.h"
#include "ui_personnel.h"
#include "barchart.h"
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QTableWidgetItem>
#include <QSettings>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>
#include <QFile>
#include <QMessageBox>
#include <QDialog>
#include <QComboBox>
#include <QDateEdit>
#include <QDialogButtonBox>
#include <QTableWidget>
#include <QTextEdit>
#include <QDir>
#include <QDate>
#include <QFileDialog>
#include <QFileInfo>
#include <QPageLayout>
#include <QPageSize>
#include <QPdfWriter>
#include <QPushButton>
#include <QRegularExpression>
#include <QTextDocument>
#include <QDesktopServices>
#include <QApplication>
#include <QStyle>
#include <QUrl>
#include <algorithm>

static QWidget *pastille(const QString &texte, const QString &couleur) {
    auto *c=new QWidget; c->setStyleSheet("background:transparent;"); auto *l=new QHBoxLayout(c); l->setContentsMargins(4,4,4,4);
    auto *x=new QLabel(texte); x->setAlignment(Qt::AlignCenter); x->setStyleSheet(QString("background:%1;color:white;border-radius:4px;padding:2px 4px;font-weight:bold;font-size:8pt;").arg(couleur)); l->addWidget(x); return c;
}
static QString dataFile() { return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)+"/employes.json"; }

Personnel::Personnel(QWidget *parent):QWidget(parent),ui(new Ui::Personnel){
    ui->setupUi(this);
    ui->labelLogo->setText("◈  WaterFlow");
    ui->menuLateral->setCurrentRow(1);
    ui->menuLateral->setFocusPolicy(Qt::NoFocus);
    const QList<QStyle::StandardPixmap> menuIcons = {
        QStyle::SP_ComputerIcon, QStyle::SP_DirHomeIcon, QStyle::SP_DriveHDIcon,
        QStyle::SP_FileDialogDetailedView, QStyle::SP_ArrowRight, QStyle::SP_DialogOpenButton,
        QStyle::SP_FileDialogInfoView, QStyle::SP_MessageBoxWarning
    };
    for (int i=0; i<ui->menuLateral->count() && i<menuIcons.size(); ++i) {
        ui->menuLateral->item(i)->setIcon(QApplication::style()->standardIcon(menuIcons[i]));
        ui->menuLateral->item(i)->setSizeHint(QSize(0,44));
    }
    ui->btnAjouter->setIcon(QApplication::style()->standardIcon(QStyle::SP_FileDialogNewFolder));
    ui->btnEnregistrer->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogSaveButton));
    ui->btnSupprimer->setIcon(QApplication::style()->standardIcon(QStyle::SP_TrashIcon));
    ui->btnExporter->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogSaveButton));
    connect(ui->btnAjouter,&QPushButton::clicked,this,&Personnel::ajouterEmploye);
    connect(ui->btnEnregistrer,&QPushButton::clicked,this,&Personnel::enregistrerEmploye);
    connect(ui->btnSupprimer,&QPushButton::clicked,this,&Personnel::supprimerEmploye);
    connect(ui->tableEmployes,&QTableWidget::cellClicked,this,&Personnel::selectionEmploye);
    connect(ui->lineEditRecherche,&QLineEdit::textChanged,this,&Personnel::rechercher);
    connect(ui->comboDepartement,QOverload<int>::of(&QComboBox::currentIndexChanged),this,&Personnel::rechercher);
    connect(ui->comboTri,QOverload<int>::of(&QComboBox::currentIndexChanged),this,&Personnel::rechercher);
    connect(ui->btnExporter,&QPushButton::clicked,this,&Personnel::exporterAttestation);
    connect(ui->menuLateral,&QListWidget::currentRowChanged,this,&Personnel::menuClique);
    chargerDonnees(); rafraichirTable(); rafraichirPlanning(); rafraichirGraphique(); viderFormulaire();
}
Personnel::~Personnel(){delete ui;}

void Personnel::chargerDonnees(){
    QFile f(dataFile()); if(!f.open(QIODevice::ReadOnly)) return; const auto a=QJsonDocument::fromJson(f.readAll()).array();
    for(const auto &v:a){auto o=v.toObject(); Employe e; e.id=o["id"].toString(); e.nom=o["nom"].toString(); e.prenom=o["prenom"].toString(); e.poste=o["poste"].toString(); e.departement=o["departement"].toString(); e.dateEmbauche=o["date"].toString(); e.telephone=o["tel"].toString(); e.shift=o["shift"].toString(); e.statut=o["statut"].toString(); m_employes.append(e);}
}
void Personnel::sauvegarderDonnees() const{
    QDir().mkpath(QFileInfo(dataFile()).absolutePath()); QJsonArray a; for(const auto&e:m_employes){QJsonObject o; o["id"]=e.id;o["nom"]=e.nom;o["prenom"]=e.prenom;o["poste"]=e.poste;o["departement"]=e.departement;o["date"]=e.dateEmbauche;o["tel"]=e.telephone;o["shift"]=e.shift;o["statut"]=e.statut;a.append(o);} QFile f(dataFile()); if(!f.open(QIODevice::WriteOnly|QIODevice::Truncate) || f.write(QJsonDocument(a).toJson())<0) QMessageBox::warning(nullptr,"Enregistrement impossible","Les données n'ont pas pu être enregistrées dans :\n"+dataFile());
}
QString Personnel::prochainId() const { int max=0; for(const auto&e:m_employes){bool ok=false;QString id=e.id; int n=id.remove("EMP-").toInt(&ok);if(ok)max=qMax(max,n);} return QString("EMP-%1").arg(max+1,3,10,QChar('0')); }
void Personnel::rafraichirTable(){
    auto*t=ui->tableEmployes; t->setSortingEnabled(false); t->clearContents(); t->setRowCount(0); QString q=ui->lineEditRecherche->text().trimmed().toLower(); QString dep=ui->comboDepartement->currentText();
    const bool tousDepartements = (ui->comboDepartement->currentIndex() == 0);
    QList<int> idx; for(int i=0;i<m_employes.size();++i){const auto&e=m_employes[i]; if(!q.isEmpty() && !(e.nom+" "+e.prenom+" "+e.poste+" "+e.id).toLower().contains(q))continue; if(!tousDepartements && e.departement!=dep)continue; idx<<i;}
    const QString tri=ui->comboTri->currentText(); std::sort(idx.begin(),idx.end(),[&](int a,int b){const auto&A=m_employes[a],&B=m_employes[b]; if(tri.contains("Z → A")) return A.nom.toLower()>B.nom.toLower(); if(tri.contains("nom")) return A.nom.toLower()<B.nom.toLower(); return A.id<B.id;});
    t->setRowCount(idx.size()); t->verticalHeader()->setDefaultSectionSize(34); t->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents); t->horizontalHeader()->setSectionResizeMode(1,QHeaderView::Stretch); t->horizontalHeader()->setSectionResizeMode(2,QHeaderView::Stretch);
    for(int r=0;r<idx.size();++r){const auto&e=m_employes[idx[r]]; QStringList v={e.id,e.nom,e.prenom,e.poste,e.departement,e.shift};for(int c=0;c<v.size();++c)t->setItem(r,c,new QTableWidgetItem(v[c]));QString col=e.statut=="Suspendu"?"#E74C3C":e.statut=="En congé"?"#F39C12":"#2ECC71";t->setCellWidget(r,6,pastille(e.statut,col)); t->item(r,0)->setData(Qt::UserRole,idx[r]);}
    ui->labelInfo->setText(QString("Responsable RH · %1 employé(s)").arg(m_employes.size()));
}
void Personnel::rafraichirPlanning(){
    auto*t=ui->tablePlanning;t->setRowCount(m_employes.size());t->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);t->horizontalHeader()->setSectionResizeMode(0,QHeaderView::ResizeToContents);
    QStringList days={"Lun","Mar","Mer","Jeu","Ven"}; for(int r=0;r<m_employes.size();++r){auto&e=m_employes[r];t->setItem(r,0,new QTableWidgetItem(e.prenom+" "+e.nom));for(int c=1;c<=5;++c){QString s=e.statut=="Suspendu"||e.statut=="En congé"?"Absent":e.shift;QString col=s=="Matin"?"#00B4D8":s=="Après-midi"?"#0A4D8C":s=="Nuit"?"#5D6D7E":"#E74C3C";t->setCellWidget(r,c,pastille(s,col));}}
}
void Personnel::rafraichirGraphique(){
    while(auto*i=ui->layoutGraphique->takeAt(0)){delete i->widget();delete i;} QStringList labs={"Production","Maintenance","Logistique","Qualité"};QList<int> vals;for(auto&d:labs){int n=0;for(auto&e:m_employes)if(e.departement==d)n++;vals<<n;}auto*g=new BarChart;g->setData(labs,vals);ui->layoutGraphique->addWidget(g);
}
void Personnel::viderFormulaire(){m_editingIndex=-1;ui->lineEditNom->clear();ui->lineEditPrenom->clear();ui->lineEditPoste->clear();ui->lineEditTelephone->clear();ui->dateEditEmbauche->setDate(QDate::currentDate());ui->comboDepartementFiche->setCurrentIndex(0);ui->comboShift->setCurrentIndex(0);ui->comboStatut->setCurrentIndex(0);ui->labelTitreFiche->setText("Fiche employé — Nouveau");ui->tableEmployes->clearSelection();}
void Personnel::remplirFormulaire(int row){if(row<0||row>=ui->tableEmployes->rowCount())return;int i=ui->tableEmployes->item(row,0)->data(Qt::UserRole).toInt();m_editingIndex=i;const auto&e=m_employes[i];ui->lineEditNom->setText(e.nom);ui->lineEditPrenom->setText(e.prenom);ui->lineEditPoste->setText(e.poste);ui->comboDepartementFiche->setCurrentText(e.departement);ui->dateEditEmbauche->setDate(QDate::fromString(e.dateEmbauche,"dd/MM/yyyy"));ui->lineEditTelephone->setText(e.telephone);ui->comboShift->setCurrentText(e.shift);ui->comboStatut->setCurrentText(e.statut);ui->labelTitreFiche->setText("Fiche employé — "+e.id);}
void Personnel::selectionEmploye(int row){remplirFormulaire(row);}
void Personnel::ajouterEmploye(){m_editingIndex=-1;ui->lineEditNom->clear();ui->lineEditPrenom->clear();ui->lineEditPoste->clear();ui->lineEditTelephone->clear();ui->dateEditEmbauche->setDate(QDate::currentDate());ui->comboDepartementFiche->setCurrentIndex(0);ui->comboShift->setCurrentIndex(0);ui->comboStatut->setCurrentIndex(0);ui->labelTitreFiche->setText("Fiche employé — Nouveau");ui->tableEmployes->clearSelection();ui->tableEmployes->setCurrentItem(nullptr);ui->lineEditNom->setFocus();}
void Personnel::enregistrerEmploye(){
    const QString nom=ui->lineEditNom->text().trimmed(), prenom=ui->lineEditPrenom->text().trimmed(); if(nom.isEmpty()||prenom.isEmpty()){QMessageBox::warning(this,"Données manquantes","Le nom et le prénom sont obligatoires.");return;}
    int current=m_editingIndex;
    if(current<0 || current>=m_employes.size()) current=-1;
    static const QRegularExpression reTel("^\\+?[0-9\\s.()-]+$");
    const QString tel=ui->lineEditTelephone->text().trimmed();
    if(!tel.isEmpty()){
        int chiffres=0; for(const QChar &c:tel) if(c.isDigit()) ++chiffres;
        if(!reTel.match(tel).hasMatch()||chiffres<8||chiffres>15){QMessageBox::warning(this,"Téléphone invalide","Le téléphone doit contenir de 8 à 15 chiffres (ex. 71 123 456 ou +216 71 123 456).");ui->lineEditTelephone->setFocus();return;}
    }
    if(ui->dateEditEmbauche->date()>QDate::currentDate()){QMessageBox::warning(this,"Date invalide","La date d'embauche ne peut pas être dans le futur.");return;}
    for(int i=0;i<m_employes.size();++i){
        if(i!=current && m_employes[i].nom.compare(nom,Qt::CaseInsensitive)==0 && m_employes[i].prenom.compare(prenom,Qt::CaseInsensitive)==0){
            QMessageBox::warning(this,"Doublon",QString("%1 %2 existe déjà (%3).").arg(nom,prenom,m_employes[i].id));return;
        }
    }
    Employe e; e.id=current>=0?m_employes[current].id:prochainId();e.nom=nom;e.prenom=prenom;e.poste=ui->lineEditPoste->text().trimmed();e.departement=ui->comboDepartementFiche->currentText();e.dateEmbauche=ui->dateEditEmbauche->date().toString("dd/MM/yyyy");e.telephone=ui->lineEditTelephone->text().trimmed();e.shift=ui->comboShift->currentText();e.statut=ui->comboStatut->currentText();
    if(current>=0)m_employes[current]=e;else { m_employes.append(e); current=m_employes.size()-1; } sauvegarderDonnees(); ui->lineEditRecherche->clear(); ui->comboDepartement->setCurrentIndex(0); rafraichirTable(); rafraichirPlanning(); rafraichirGraphique(); for(int r=0;r<ui->tableEmployes->rowCount();++r){ if(ui->tableEmployes->item(r,0) && ui->tableEmployes->item(r,0)->text()==e.id){ ui->tableEmployes->selectRow(r); ui->tableEmployes->scrollToItem(ui->tableEmployes->item(r,0)); remplirFormulaire(r); break; } } QMessageBox::information(this,"Enregistré","Les informations ont été enregistrées et ajoutées à la liste.");
}
void Personnel::supprimerEmploye(){
    const int i=indexSelectionne();
    if(i<0){QMessageBox::information(this,"Sélection","Sélectionnez un employé à supprimer.");return;}
    const Employe cible=m_employes[i];
    QMessageBox box(QMessageBox::Question,"Confirmation",
        QString("Supprimer définitivement %1 %2 (%3) ?\nCette action est irréversible.").arg(cible.prenom,cible.nom,cible.id),
        QMessageBox::NoButton,this);
    QPushButton *oui=box.addButton("Supprimer",QMessageBox::AcceptRole);
    QPushButton *non=box.addButton("Annuler",QMessageBox::RejectRole);
    box.setDefaultButton(non);
    box.exec();
    if(box.clickedButton()!=oui) return;
    m_employes.removeAt(i);
    m_editingIndex=-1;
    sauvegarderDonnees();
    rafraichirTable(); rafraichirPlanning(); rafraichirGraphique(); viderFormulaire();
}
void Personnel::rechercher(){rafraichirTable();}
int Personnel::indexSelectionne() const{
    const auto sel=ui->tableEmployes->selectionModel()->selectedRows();
    if(sel.isEmpty()) return -1;
    const QTableWidgetItem *it=ui->tableEmployes->item(sel.first().row(),0);
    if(!it) return -1;
    const int i=it->data(Qt::UserRole).toInt();
    return (i>=0 && i<m_employes.size()) ? i : -1;
}

QString Personnel::htmlAttestation(const Employe &e, const QString &reference) const{
    auto h=[](const QString &s){return s.toHtmlEscaped();};
    const QString nomComplet=e.prenom+" "+e.nom.toUpper();
    const QDate embauche=QDate::fromString(e.dateEmbauche,"dd/MM/yyyy");

    QString html;
    html+="<html><body style=\"font-size:12pt;\">";
    html+="<p><span style=\"font-size:20pt; font-weight:bold; color:#0A4D8C;\">WaterFlow</span><br/>"
          "<span style=\"color:#5B6B7B;\">Service des Ressources Humaines</span></p>";
    html+="<hr/>";
    html+="<p align=\"right\">Réf. : "+h(reference)+"</p>";
    html+="<br/><br/>";
    html+="<p align=\"center\"><span style=\"font-size:22pt; font-weight:bold; color:#0A4D8C;\">ATTESTATION DE PRÉSENCE</span></p>";
    html+="<br/><br/>";
    html+="<p align=\"justify\" style=\"line-height:150%;\">Je soussigné(e), responsable des Ressources Humaines "
          "de la société <b>WaterFlow</b>, atteste par la présente que :</p>";
    html+="<p align=\"center\" style=\"font-size:15pt;\"><b>"+h(nomComplet)+"</b></p>";
    html+="<p align=\"justify\" style=\"line-height:150%;\">matricule <b>"+h(e.id)+"</b>, occupe le poste de <b>"+h(e.poste)
          +"</b> au sein du département <b>"+h(e.departement)+"</b>";
    if(embauche.isValid())
        html+=", et fait partie de notre personnel depuis le <b>"+embauche.toString("dd/MM/yyyy")+"</b>";
    html+=".</p>";
    html+="<p align=\"justify\" style=\"line-height:150%;\">L'intéressé(e) est actuellement <b>en activité</b> "
          "et <b>présent(e) à son poste</b>";
    if(!e.shift.isEmpty())
        html+=", affecté(e) au shift <b>"+h(e.shift.toLower())+"</b>";
    html+=".</p>";
    html+="<p align=\"justify\" style=\"line-height:150%;\">Cette attestation est délivrée à l'intéressé(e) pour servir et valoir ce que de droit.</p>";
    html+="<br/><br/>";
    html+="<p>Fait à ______________________, le <b>"+QDate::currentDate().toString("dd/MM/yyyy")+"</b></p>";
    html+="<br/>";
    html+="<p align=\"right\">Le Responsable des Ressources Humaines<br/><br/><br/><br/>Signature et cachet</p>";
    html+="</body></html>";
    return html;
}

void Personnel::exporterAttestation(){
    const int i=indexSelectionne();
    if(i<0){
        QMessageBox::information(this,"Attestation de présence","Sélectionnez d'abord un employé dans la liste pour générer son attestation de présence.");
        return;
    }
    const Employe e=m_employes[i];
    if(e.statut!="Actif"){
        QMessageBox::warning(this,"Attestation de présence",
            QString("%1 %2 est actuellement « %3 » : une attestation de présence ne peut être délivrée que pour un employé au statut « Actif ».")
                .arg(e.prenom,e.nom,e.statut));
        return;
    }

    QString idSansTiret=e.id; idSansTiret.remove('-');
    const QString reference=QString("ATT-%1-%2").arg(idSansTiret,QDate::currentDate().toString("yyyyMMdd"));
    const QString suggestion=QDir::homePath()+"/attestation_presence_"+e.nom.simplified().replace(' ','_')+"_"+e.prenom.simplified().replace(' ','_')+".pdf";
    QString chemin=QFileDialog::getSaveFileName(this,"Enregistrer l'attestation de présence",suggestion,"Documents PDF (*.pdf)");
    if(chemin.isEmpty()) return;                        // annulé
    if(!chemin.endsWith(".pdf",Qt::CaseInsensitive)) chemin+=".pdf";

    {   // vérifie qu'on peut écrire à cet endroit avant de générer le document
        QFile test(chemin);
        if(!test.open(QIODevice::WriteOnly)){
            QMessageBox::critical(this,"Attestation de présence","Impossible d'écrire dans ce fichier (est-il ouvert dans un autre programme ?) :\n"+QDir::toNativeSeparators(chemin));
            return;
        }
    }

    QTextDocument doc;
    doc.setHtml(htmlAttestation(e,reference));
    {
        QPdfWriter writer(chemin);
        writer.setPageSize(QPageSize(QPageSize::A4));
        writer.setPageOrientation(QPageLayout::Portrait);
        writer.setPageMargins(QMarginsF(20,20,20,20),QPageLayout::Millimeter);
        writer.setTitle("Attestation de présence - "+e.prenom+" "+e.nom);
        writer.setCreator("WaterFlow");
        doc.print(&writer);
    }                                                   // le fichier est finalisé ici

    if(QFileInfo(chemin).size()<=0){
        QMessageBox::critical(this,"Attestation de présence","La création du PDF a échoué :\n"+QDir::toNativeSeparators(chemin));
        return;
    }

    QMessageBox box(QMessageBox::Information,"Attestation de présence",
                    QString("L'attestation de présence a été créée :\n%1").arg(QDir::toNativeSeparators(chemin)),
                    QMessageBox::NoButton,this);
    QPushButton *ouvrir=box.addButton("Ouvrir le PDF",QMessageBox::AcceptRole);
    box.addButton("Fermer",QMessageBox::RejectRole);
    box.exec();
    if(box.clickedButton()==ouvrir) QDesktopServices::openUrl(QUrl::fromLocalFile(chemin));
}

void Personnel::menuClique(int row){if(row==1)return;QStringList names={"Tableau de bord","Personnel","Équipements","Production & stocks","Commandes","Livraisons","Statistiques","Alertes"};QMessageBox::information(this,names.value(row),"Cette section est prête pour être reliée à son module.\nLa gestion du personnel reste accessible via Personnel.");ui->menuLateral->setCurrentRow(1);}
void Personnel::deconnecter(){close();}
