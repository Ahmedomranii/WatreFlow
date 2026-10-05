#pragma once
#include <QWidget>
#include <QList>
#include <QString>

class QTableWidgetItem;
namespace Ui { class Personnel; }

struct Employe {
    QString id, nom, prenom, poste, departement, dateEmbauche, telephone, shift, statut;
};

class Personnel : public QWidget
{
    Q_OBJECT
public:
    explicit Personnel(QWidget *parent = nullptr);
    ~Personnel();

private slots:
    void ajouterEmploye();
    void enregistrerEmploye();
    void supprimerEmploye();
    void selectionEmploye(int row);
    void rechercher();
    void gestionCongesAbsences();
    void menuClique(int row);
    void deconnecter();

private:
    void chargerDonnees();
    void sauvegarderDonnees() const;
    void rafraichirTable();
    void rafraichirPlanning();
    void rafraichirGraphique();
    void remplirFormulaire(int row);
    void viderFormulaire();
    QString prochainId() const;
    QList<Employe> m_employes;
    int m_editingIndex = -1;
    Ui::Personnel *ui;
};
