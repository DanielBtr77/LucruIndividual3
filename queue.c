#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ============================================================
// STRUCTURI
// ============================================================

typedef struct Document {
    int id;
    char nume[100];
    int total_randuri;
    int randuri_ramase;
    struct Document *urmator;
} Document;

typedef struct Printer {
    int id;
    int timp_rand;         // timp pentru un rand (unitati)
    int timp_ramas;        // cat mai trebuie sa astepte pana termina randul curent
    Document *doc_curent;  // documentul care se tipareste acum
    struct Printer *urmator;
} Printer;

// Liste globale
Document *coada_cap = NULL;   // coada de documente in asteptare
Document *coada_coada = NULL;
Printer  *lista_printere = NULL;

int id_doc_contor = 1;
int id_printer_contor = 1;

// ============================================================
// FUNCTII DOCUMENT
// ============================================================

Document* creeazaDocument(const char *nume, int randuri) {
    Document *d = (Document*)malloc(sizeof(Document));
    if (!d) { printf("Eroare memorie!\n"); return NULL; }
    d->id = id_doc_contor++;
    strncpy(d->nume, nume, 99);
    d->total_randuri = randuri;
    d->randuri_ramase = randuri;
    d->urmator = NULL;
    return d;
}

void adaugaDocumentInCoada(const char *nume, int randuri) {
    Document *d = creeazaDocument(nume, randuri);
    if (!d) return;

    if (coada_coada == NULL) {
        coada_cap = d;
        coada_coada = d;
    } else {
        coada_coada->urmator = d;
        coada_coada = d;
    }
    printf("Document adaugat: [ID:%d | %s | %d randuri]\n", d->id, d->nume, d->total_randuri);
}

Document* scoateDocumentDinCoada() {
    if (coada_cap == NULL) return NULL;
    Document *d = coada_cap;
    coada_cap = coada_cap->urmator;
    if (coada_cap == NULL) coada_coada = NULL;
    d->urmator = NULL;
    return d;
}

// ============================================================
// FUNCTII PRINTER
// ============================================================

void adaugaPrinter(int timp_rand) {
    Printer *p = (Printer*)malloc(sizeof(Printer));
    if (!p) { printf("Eroare memorie!\n"); return; }
    p->id = id_printer_contor++;
    p->timp_rand = timp_rand;
    p->timp_ramas = 0;
    p->doc_curent = NULL;
    p->urmator = NULL;

    // adaugam la sfarsitul listei
    if (lista_printere == NULL) {
        lista_printere = p;
    } else {
        Printer *curent = lista_printere;
        while (curent->urmator != NULL) curent = curent->urmator;
        curent->urmator = p;
    }
    printf("Imprimanta adaugata: [ID:%d | %d unitati/rand]\n", p->id, p->timp_rand);
}

void scoatePrinter(int id) {
    Printer *curent = lista_printere;
    Printer *anterior = NULL;

    while (curent != NULL) {
        if (curent->id == id) {
            // daca are document in curs, il returnam in coada
            if (curent->doc_curent != NULL) {
                printf("Imprimanta %d defectata! Documentul '%s' returnat in coada.\n",
                       id, curent->doc_curent->nume);
                // punem documentul inapoi la inceputul cozii
                curent->doc_curent->urmator = coada_cap;
                coada_cap = curent->doc_curent;
                if (coada_coada == NULL) coada_coada = coada_cap;
                curent->doc_curent = NULL;
            }

            if (anterior == NULL)
                lista_printere = curent->urmator;
            else
                anterior->urmator = curent->urmator;

            free(curent);
            printf("Imprimanta %d scoasa din functiune.\n", id);
            return;
        }
        anterior = curent;
        curent = curent->urmator;
    }
    printf("Imprimanta cu ID %d nu a fost gasita.\n", id);
}

// ============================================================
// DISTRIBUTIE DOCUMENTE CATRE IMPRIMANTE LIBERE
// Algoritmul aloca documentul catre imprimanta cu cel mai mic
// timp de tiparire (minimizeaza timpul total de asteptare)
// ============================================================

void distribuieDocumente() {
    if (lista_printere == NULL) {
        printf("Nu exista imprimante disponibile!\n");
        return;
    }

    Printer *p = lista_printere;
    while (p != NULL) {
        // imprimanta e libera si exista documente in coada
        if (p->doc_curent == NULL && coada_cap != NULL) {
            p->doc_curent = scoateDocumentDinCoada();
            p->timp_ramas = p->timp_rand; // timp pentru primul rand
            printf("  -> Documentul '%s' (ID:%d) alocat la Imprimanta %d\n",
                   p->doc_curent->nume, p->doc_curent->id, p->id);
        }
        p = p->urmator;
    }
}

// ============================================================
// SIMULARE O UNITATE DE TIMP
// ============================================================

void simuleazaPas() {
    Printer *p = lista_printere;

    while (p != NULL) {
        if (p->doc_curent != NULL) {
            p->timp_ramas--;

            if (p->timp_ramas <= 0) {
                // s-a terminat un rand
                p->doc_curent->randuri_ramase--;

                if (p->doc_curent->randuri_ramase <= 0) {
                    // document terminat
                    printf("  [Imprimanta %d] Document '%s' (ID:%d) FINALIZAT!\n",
                           p->id, p->doc_curent->nume, p->doc_curent->id);
                    free(p->doc_curent);
                    p->doc_curent = NULL;
                } else {
                    // mai sunt randuri
                    p->timp_ramas = p->timp_rand;
                }
            }
        }
        p = p->urmator;
    }

    // dupa fiecare pas, distribuim documente noi la imprimantele libere
    distribuieDocumente();
}

// ============================================================
// BONUS: ANULARE DOCUMENT
// ============================================================

void anuleazaDocument(int id) {
    // cautam in coada de asteptare
    Document *curent = coada_cap;
    Document *anterior = NULL;

    while (curent != NULL) {
        if (curent->id == id) {
            if (anterior == NULL)
                coada_cap = curent->urmator;
            else
                anterior->urmator = curent->urmator;

            if (curent == coada_coada)
                coada_coada = anterior;

            printf("Document '%s' (ID:%d) anulat din coada de asteptare.\n",
                   curent->nume, curent->id);
            free(curent);
            return;
        }
        anterior = curent;
        curent = curent->urmator;
    }

    // cautam la imprimante (document in curs de tiparire)
    Printer *p = lista_printere;
    while (p != NULL) {
        if (p->doc_curent != NULL && p->doc_curent->id == id) {
            printf("Document '%s' (ID:%d) anulat de pe Imprimanta %d.\n",
                   p->doc_curent->nume, p->doc_curent->id, p->id);
            free(p->doc_curent);
            p->doc_curent = NULL;
            return;
        }
        p = p->urmator;
    }

    printf("Documentul cu ID %d nu a fost gasit.\n", id);
}

// ============================================================
// AFISARE STARE SISTEM
// ============================================================

void afisareStare() {
    printf("\n---------- STARE SISTEM ----------\n");

    printf("IMPRIMANTE:\n");
    Printer *p = lista_printere;
    if (p == NULL) printf("  (nicio imprimanta)\n");
    while (p != NULL) {
        if (p->doc_curent != NULL) {
            printf("  Imprimanta %d [%d u/rand]: tipareste '%s' (%d/%d randuri ramase)\n",
                   p->id, p->timp_rand,
                   p->doc_curent->nume,
                   p->doc_curent->randuri_ramase,
                   p->doc_curent->total_randuri);
        } else {
            printf("  Imprimanta %d [%d u/rand]: LIBERA\n", p->id, p->timp_rand);
        }
        p = p->urmator;
    }

    printf("COADA DE ASTEPTARE:\n");
    Document *d = coada_cap;
    if (d == NULL) printf("  (coada goala)\n");
    while (d != NULL) {
        printf("  [ID:%d] %s (%d randuri)\n", d->id, d->nume, d->randuri_ramase);
        d = d->urmator;
    }
    printf("----------------------------------\n\n");
}

// ============================================================
// ELIBERARE MEMORIE
// ============================================================

void elibereazaMemorie() {
    // eliberam coada
    Document *d = coada_cap;
    while (d != NULL) {
        Document *tmp = d;
        d = d->urmator;
        free(tmp);
    }
    coada_cap = NULL;
    coada_coada = NULL;

    // eliberam imprimantele si documentele lor curente
    Printer *p = lista_printere;
    while (p != NULL) {
        if (p->doc_curent != NULL) free(p->doc_curent);
        Printer *tmp = p;
        p = p->urmator;
        free(tmp);
    }
    lista_printere = NULL;

    printf("Memoria a fost eliberata.\n");
}

// ============================================================
// MAIN - DEMONSTRATIE
// ============================================================

int main() {
    printf("=== Sistem de Gestiune a Cozii de Tiparire ===\n\n");

    // initializam 3 imprimante cu timpi diferiti
    printf("--- Initializare imprimante ---\n");
    adaugaPrinter(2);  // imprimanta 1: 2 unitati per rand (rapida)
    adaugaPrinter(3);  // imprimanta 2: 3 unitati per rand
    adaugaPrinter(5);  // imprimanta 3: 5 unitati per rand (lenta)

    // adaugam documente in coada
    printf("\n--- Adaugare documente ---\n");
    adaugaDocumentInCoada("Raport_Lunar.txt", 4);
    adaugaDocumentInCoada("Factura_001.pdf", 2);
    adaugaDocumentInCoada("Contract.docx", 6);
    adaugaDocumentInCoada("Prezentare.ppt", 3);
    adaugaDocumentInCoada("Email_urgenta.txt", 1);

    // distribuim documentele initiale
    printf("\n--- Distributie initiala ---\n");
    distribuieDocumente();

    afisareStare();

    // simulam 5 pasi de timp
    printf("--- Simulare tiparire (10 pasi) ---\n");
    for (int pas = 1; pas <= 10; pas++) {
        printf("\n[Pas %d]\n", pas);
        simuleazaPas();
    }

    afisareStare();

    // demonstram BONUS: anulare document
    printf("--- BONUS: Adaugam documente noi si anulam unul ---\n");
    adaugaDocumentInCoada("Document_de_anulat.txt", 5);
    adaugaDocumentInCoada("Alt_document.txt", 2);
    distribuieDocumente();
    afisareStare();
    anuleazaDocument(6); // anulam documentul cu ID 6

    // demonstram BONUS: scoatere imprimanta defectata
    printf("\n--- BONUS: Imprimanta 2 se defecteaza ---\n");
    scoatePrinter(2);

    // demonstram BONUS: adaugare imprimanta noua
    printf("\n--- BONUS: Adaugare imprimanta noua ---\n");
    adaugaPrinter(1); // imprimanta foarte rapida
    distribuieDocumente();

    afisareStare();

    // eliberam memoria
    printf("\n--- Eliberare memorie ---\n");
    elibereazaMemorie();

    return 0;
}
