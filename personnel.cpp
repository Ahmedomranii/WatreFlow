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
#include <algorithm>

static QWidget *pastille(const QString &texte, const QString &couleur) {
    auto *c=new QWidget; c->setStyleSheet("background:transparent;"); auto *l=new QHBoxLayout(c); l->setContentsMargins(4,4,4,4);
    auto *x=new QLabel(texte); x->setAlignment(Qt::AlignCenter); x->setStyleSheet(QString("background:%1;color:white;border-radius:4px;padding:2px 4px;font-weight:bold;font-size:8pt;").arg(couleur)); l->addWidget(x); return c;
}
static QString dataFile() { return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)+"/employes.json"; }

Personnel::Personnel(QWidget *parent):QWidget(parent),ui(new Ui::Personnel){
    ui->setupUi(this); ui->menuLateral->setCurrentRow(1); ui->menuLateral->setFocusPolicy(Qt::NoFocus);
    connect(ui->btnAjouter,&QPushButton::clicked,this,&Personnel::ajouterEmploye);
    connect(ui->btnEnregistrer,&QPushButton::clicked,this,&Personnel::enregistrerEmploye);
    connect(ui->btnSupprimer,&QPushButton::clicked,this,&Personnel::supprimerEmploye);
    connect(ui->tableEmployes,&QTableWidget::cellClicked,this,&Personnel::selectionEmploye);
    connect(ui->lineEditRecherche,&QLineEdit::textChanged,this,&Personnel::rechercher);
    connect(ui->comboDepartement,QOverload<int>::of(&QComboBox::currentIndexChanged),this,&Personnel::rechercher);
    connect(ui->comboTri,QOverload<int>::of(&QComboBox::currentIndexChanged),this,&Personnel::rechercher);
    connect(ui->btnExporter,&QPushButton::clicked,this,&Personnel::gestionCongesAbsences);
    connect(ui->menuLateral,&QListWidget::currentRowChanged,this,&Personnel::menuClique);
    chargerDonnees(); rafraichirTable(); rafraichirPlanning(); rafraichirGraphique(); viderFormulaire();
}
Personnel::~Personnel(){delete ui;}

void Personnel::chargerDonnees(){
    QFile f(dataFile()); if(!f.open(QIODevice::ReadOnly)) return; const auto a=QJsonDocument::fromJson(f.readAll()).array();
    for(const auto &v:a){auto o=v.toObject(); Employe e; e.id=o["id"].toString(); e.nom=o["nom"].toString(); e.prenom=o["prenom"].toString(); e.poste=o["poste"].toString(); e.departement=o["departement"].toString(); e.dateEmbauche=o["date"].toString(); e.telephone=o["tel"].toString(); e.shift=o["shift"].toString(); e.statut=o["statut"].toString(); m_employes.append(e);}
}
void Personnel::sauvegarderDonnees() const{
    QDir().mkpath(QFileInfo(dataFile()).absolutePath()); QJsonArray a; for(const auto&e:m_employes){QJsonObject o; o["id"]=e.id;o["nom"]=e.nom;o["prenom"]=e.prenom;o["poste"]=e.poste;o["departement"]=e.departement;o["date"]=e.dateEmbauche;o["tel"]=e.telephone;o["shift"]=e.shift;o["statut"]=e.statut;a.append(o);} QFile f(dataFile()); if(f.open(QIODevice::WriteOnly|QIODevice::Truncate)) f.write(QJsonDocument(a).toJson());
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
    Employe e; e.id=current>=0?m_employes[current].id:prochainId();e.nom=nom;e.prenom=prenom;e.poste=ui->lineEditPoste->text().trimmed();e.departement=ui->comboDepartementFiche->currentText();e.dateEmbauche=ui->dateEditEmbauche->date().toString("dd/MM/yyyy");e.telephone=ui->lineEditTelephone->text().trimmed();e.shift=ui->comboShift->currentText();e.statut=ui->comboStatut->currentText();
    if(current>=0)m_employes[current]=e;else { m_employes.append(e); current=m_employes.size()-1; } sauvegarderDonnees(); ui->lineEditRecherche->clear(); ui->comboDepartement->setCurrentIndex(0); rafraichirTable(); rafraichirPlanning(); rafraichirGraphique(); m_editingIndex=-1; ui->labelTitreFiche->setText("Fiche employé — Nouveau"); for(int r=0;r<ui->tableEmployes->rowCount();++r){ if(ui->tableEmployes->item(r,0) && ui->tableEmployes->item(r,0)->text()==e.id){ ui->tableEmployes->selectRow(r); ui->tableEmployes->scrollToItem(ui->tableEmployes->item(r,0)); break; } } QMessageBox::information(this,"Enregistré","Les informations ont été enregistrées et ajoutées à la liste.");
}
void Personnel::supprimerEmploye(){auto sel=ui->tableEmployes->selectionModel()->selectedRows();if(sel.isEmpty()){QMessageBox::information(this,"Sélection","Sélectionnez un employé à supprimer.");return;}int row=sel.first().row(),i=ui->tableEmployes->item(row,0)->data(Qt::UserRole).toInt();if(QMessageBox::question(this,"Confirmation","Supprimer cet employé ?")==QMessageBox::Yes){m_employes.removeAt(i);m_editingIndex=-1;sauvegarderDonnees();rafraichirTable();rafraichirPlanning();rafraichirGraphique();viderFormulaire();}}
void Personnel::rechercher(){rafraichirTable();}
void Personnel::gestionCongesAbsences(){
    QDialog dlg(this);
    dlg.setWindowTitle("Gestion des congés et absences");
    dlg.resize(820,520);

    auto *root=new QVBoxLayout(&dlg);
    auto *title=new QLabel("Gestion des congés et absences");
    title->setStyleSheet("color:#0A4D8C;font-size:14pt;font-weight:bold;");
    root->addWidget(title);

    auto *form=new QGridLayout;
    auto *employee=new QComboBox;
    for(const auto &e:m_employes) employee->addItem(e.id+" — "+e.prenom+" "+e.nom, e.id);
    auto *type=new QComboBox;
    type->addItems({"Congé payé","Maladie","Absence justifiée","Absence non justifiée"});
    auto *debut=new QDateEdit(QDate::currentDate()); debut->setCalendarPopup(true); debut->setDisplayFormat("dd/MM/yyyy");
    auto *fin=new QDateEdit(QDate::currentDate()); fin->setCalendarPopup(true); fin->setDisplayFormat("dd/MM/yyyy");
    auto *motif=new QLineEdit; motif->setPlaceholderText("Motif (optionnel)");
    auto *ajouter=new QPushButton("Ajouter");
    form->addWidget(new QLabel("Employé"),0,0); form->addWidget(employee,0,1);
    form->addWidget(new QLabel("Type"),0,2); form->addWidget(type,0,3);
    form->addWidget(new QLabel("Du"),1,0); form->addWidget(debut,1,1);
    form->addWidget(new QLabel("Au"),1,2); form->addWidget(fin,1,3);
    form->addWidget(new QLabel("Motif"),2,0); form->addWidget(motif,2,1,1,3);
    form->addWidget(ajouter,3,3);
    root->addLayout(form);

    auto *table=new QTableWidget;
    table->setColumnCount(6);
    table->setHorizontalHeaderLabels({"Employé","Type","Du","Au","Motif","Statut"});
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->horizontalHeader()->setStretchLastSection(true);
    root->addWidget(table,1);

    auto *buttons=new QDialogButtonBox(QDialogButtonBox::Close);
    auto *supprimer=new QPushButton("Supprimer");
    buttons->addButton(supprimer,QDialogButtonBox::DestructiveRole);
    root->addWidget(buttons);

    struct Absence { QString id,type,debut,fin,motif,statut; };
    auto *records=new QList<Absence>;
    const QString file=QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)+"/conges_absences.json";
    auto save=[&](){ QDir().mkpath(QFileInfo(file).absolutePath()); QJsonArray a; for(const auto&r:*records){QJsonObject o; o["id"]=r.id;o["type"]=r.type;o["debut"]=r.debut;o["fin"]=r.fin;o["motif"]=r.motif;o["statut"]=r.statut;a.append(o);} QFile f(file); if(f.open(QIODevice::WriteOnly|QIODevice::Truncate)) f.write(QJsonDocument(a).toJson()); };
    auto refresh=[&](){ table->setRowCount(records->size()); for(int i=0;i<records->size();++i){const auto&r=records->at(i); QString name=r.id; for(const auto&e:m_employes) if(e.id==r.id){name=e.prenom+" "+e.nom;break;} QStringList v={name,r.type,r.debut,r.fin,r.motif,r.statut}; for(int c=0;c<v.size();++c)table->setItem(i,c,new QTableWidgetItem(v[c]));} table->resizeColumnsToContents(); };
    if(QFile f(file); f.open(QIODevice::ReadOnly)){ const auto a=QJsonDocument::fromJson(f.readAll()).array(); for(const auto&v:a){auto o=v.toObject();records->append({o["id"].toString(),o["type"].toString(),o["debut"].toString(),o["fin"].toString(),o["motif"].toString(),o["statut"].toString()});} }
    refresh();
    connect(ajouter,&QPushButton::clicked,&dlg,[&](){
        if(employee->currentIndex()<0){QMessageBox::warning(&dlg,"Aucun employé","Ajoutez d'abord un employé.");return;}
        if(fin->date()<debut->date()){QMessageBox::warning(&dlg,"Dates invalides","La date de fin doit être égale ou postérieure à la date de début.");return;}
        records->append({employee->currentData().toString(),type->currentText(),debut->date().toString("dd/MM/yyyy"),fin->date().toString("dd/MM/yyyy"),motif->text().trimmed(),"Planifié"}); save(); refresh(); motif->clear();
    });
    connect(supprimer,&QPushButton::clicked,&dlg,[&](){int row=table->currentRow(); if(row<0){QMessageBox::information(&dlg,"Sélection","Sélectionnez une ligne à supprimer.");return;} records->removeAt(row); save(); refresh();});
    connect(buttons,&QDialogButtonBox::rejected,&dlg,&QDialog::reject);
    dlg.exec();
}

void Personnel::menuClique(int row){if(row==1)return;QStringList names={"Tableau de bord","Personnel","Équipements","Production & stocks","Commandes","Livraisons","Statistiques","Alertes"};QMessageBox::information(this,names.value(row),"Cette section est prête pour être reliée à son module.\nLa gestion du personnel reste accessible via Personnel.");ui->menuLateral->setCurrentRow(1);}
void Personnel::deconnecter(){close();}
