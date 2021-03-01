#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <mpi/mpi.h>
#include <iostream>
#include <fstream>
#include <cstring>
#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>

// Numarul rankului master
const int master = 0;
// Numarul de threaduri pentru procesul master
const int master_threads = 4;
// Numarul de genuri
const int num_genres = 4;

// Extensia fisierului de iesire
const std::string output_file_extension = ".out";
// Constante pentru genuri
const std::string horror_genre = "horror";
const std::string comedy_genre = "comedy";
const std::string fantasy_genre = "fantasy";
const std::string science_fiction_genre = "science-fiction";

// Structura pentru threadurile din master
struct master_struct {
    int thread_id;
    std::string filename;
};

// Constante pentru tipurile genurile
enum genres : int {
    HORROR = 1,
    COMEDY = 2,
    FANTASY = 3,
    SCIENCE_FICTION = 4
};

// Functie de procesare a fisierului de intrare
void process_file(const std::string& genre, std::string& filename) {
    int paragraphs_count = 1;
    int lines_no = 0;
    bool is_genre = false; // Variabila folosita pentru a determina daca citim genul curent
    std::ifstream input_stream(filename);
    std::string buffer; // Buffer-ul in care retin liniile
    std::string paragraph; // Paragraful curent
    std::vector<std::string> paragraphs; // Vectorul in care retin paragrafele
    std::vector<int> paragraphs_order; // Vectorul in care retin pozitia paragrafelor
    int destination;

    // Determinarea destinatiei
    if (genre == horror_genre) {
        destination = HORROR;
    } else if (genre == comedy_genre) {
        destination = COMEDY;
    } else if (genre == fantasy_genre) {
        destination = FANTASY;
    } else if (genre == science_fiction_genre) {
        destination = SCIENCE_FICTION;
    }

    // Citesc linie cu linie din fisier
    while (getline(input_stream, buffer)) {
        /*
         * Daca citesc o linie cu numele genului, atunci setez variabila 
         * is_genre pe true pentru a putea permite salvarea paragrafului
         */
        if (buffer == genre) {
            is_genre = true;
            continue;
        }

        // Daca ma aflu in genul curent, adaug linia in paragraph
        if (is_genre) {
            if (buffer != "\0") {
                paragraph += buffer + "\n";
            }
        }

        // Daca s-a terminat paragraful
        if (buffer == "") {
            /*
             * Daca este genul curent, adaug paragraful si numarul de 
             * ordine in vectori
             */
            if (is_genre) {
                paragraphs.emplace_back(paragraph);
                paragraphs_order.emplace_back(paragraphs_count);
                paragraph.clear();
            }

            // Setez pe false variabila is_genre
            is_genre = false;
            paragraphs_count++;
        }
    }

    // Daca is_genre este true, adaug in vectori ultimul paragraf
    if (is_genre) {
        paragraphs.emplace_back(paragraph);
        paragraphs_order.emplace_back(paragraphs_count);
    }

    int genre_paragraphs = paragraphs.size();

    // Trimit numarul de paragrafe
    MPI_Send(&genre_paragraphs, 1, MPI_INT, destination, destination, MPI_COMM_WORLD);

    for (int i = 0; i < paragraphs.size(); ++i) {
        int length = paragraphs[i].size();
        char* buff = (char*) malloc((length + 1) * sizeof(char));

        // Adaug in buff paragraful
        strcpy(buff, paragraphs[i].c_str());

        // Daca ultimul caracter este '\n', se inlocuieste cu '\0'
        if (buff[length - 1] == '\n') {
            buff[length - 1] = '\0';
            length--;
        }

        // Trimit destinatiei lungimea textului
        MPI_Send(&length, 1, MPI_INT, destination, destination, MPI_COMM_WORLD);
        
        // Trimit destinatiei textul
        MPI_Send(buff, length + 1, MPI_CHAR, destination, destination, MPI_COMM_WORLD);
        
        // Trimit destinatiei numarul de ordine al paragrafului
        MPI_Send(&(paragraphs_order[i]), 1, MPI_INT, destination, destination, MPI_COMM_WORLD);

        free(buff);
    }

    // Inchiderea fisierului de intrare
    input_stream.close();
}

// Functie de procesare a paragrafelor horror
void* process_horror(void* arg) {
    int thread_id = ((master_struct*) arg)->thread_id;
    std::string filename = ((master_struct*) arg)->filename;

    process_file(horror_genre, filename);
    
    pthread_exit(NULL);
}

// Functie de procesare a paragrafelor comedy
void* process_comedy(void* arg) {
    int thread_id = ((master_struct*) arg)->thread_id;
    std::string filename = ((master_struct*) arg)->filename;
    
    process_file(comedy_genre, filename);

    pthread_exit(NULL);
}

// Functie de procesare a paragrafelor fantasy
void* process_fantasy(void* arg) {
    int thread_id = ((master_struct*) arg)->thread_id;
    std::string filename = ((master_struct*) arg)->filename;

    process_file(fantasy_genre, filename);

    pthread_exit(NULL);
}

// Functie de procesare a paragrafelor science-fiction
void* process_science_fiction(void* arg) {
    int thread_id = ((master_struct*) arg)->thread_id;
    std::string filename = ((master_struct*) arg)->filename;

    process_file(science_fiction_genre, filename);

    pthread_exit(NULL);
}

inline bool is_consonant(char& ch) {
    return  (ch >= 'b' && ch <= 'd') ||
            (ch >= 'f' && ch <= 'h') ||
            (ch >= 'j' && ch <= 'n') ||
            (ch >= 'p' && ch <= 't') ||
            (ch >= 'v' && ch <= 'z') ||
            (ch >= 'B' && ch <= 'D') ||
            (ch >= 'F' && ch <= 'H') ||
            (ch >= 'J' && ch <= 'N') ||
            (ch >= 'P' && ch <= 'T') ||
            (ch >= 'V' && ch <= 'Z');
}

int main(int argc, char *argv[]) {
    int num_tasks; // Numarul de taskuri
    int rank; // Rank-ul curent
    int provided;
    long cores = sysconf(_SC_NPROCESSORS_CONF); // Numarul de core-uri

    // Numele fisierul de intrare
    std::string input_filename(argv[1]);

    MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);
    MPI_Comm_size(MPI_COMM_WORLD, &num_tasks);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (rank == master) {
        pthread_t threads[master_threads];
        int r;
        void* status;
        // Vectori de structuri master_struct
        master_struct arguments[master_threads];

        /* 
         * Crearea a unui numar de master_threads threaduri pentru citirea din 
         * fisierul de intrare
         */
        for (int id = 0; id < master_threads; ++id) {
            arguments[id].thread_id = id;
            arguments[id].filename = input_filename;

            switch (id) {
                case 0:
                    r = pthread_create(&threads[id], NULL, process_horror, &arguments[id]);
                    break;
                
                case 1:
                    r = pthread_create(&threads[id], NULL, process_comedy, &arguments[id]);
                    break;

                case 2:
                    r = pthread_create(&threads[id], NULL, process_fantasy, &arguments[id]);
                    break;

                case 3:
                    r = pthread_create(&threads[id], NULL, process_science_fiction, &arguments[id]);
                    break;

                default:
                    break;
            }

            if (r) {
                std::cout << "Eroare la crearea thread-ului " << id << "\n";
                exit(-1);
            }
        }

        // Asteptarea threadurilor
        for (int id = 0; id < master_threads; ++id) {
            r = pthread_join(threads[id], &status);

            if (r) {
                std::cout << "Eroare la asteptarea thread-ului " << id << "\n";
                exit(-1);
            }
        }

        MPI_Status mpi_status;
        int paragraphs_no; // Numarul de paragrafe
        int paragraph_length; // Lungimea paragrafului
        int paragraph_order; // Ordinea paragrafului
        char* paragraph; // Paragraful primit

        // Numele fisierul de iesire
        std::string output_filename;

        output_filename = input_filename.substr(0, 
            input_filename.find_last_of('.')) + output_file_extension;
        
        // Stream pentru fisierul de iesire
        std::ofstream output_stream(output_filename);

        /*
         * Map in care cheia este numarul de ordine al paragrafului, iar 
         * valoarea o pereche formata din genul paragrafului si textul in sine
         */
        std::unordered_map<int, std::pair<int, std::string>> paragraphs_map;

        for (int i = 1; i <= num_genres; ++i) {
            // Primesc numarul de paragrafe
            MPI_Recv(&paragraphs_no, 1, MPI_INT, i, i, MPI_COMM_WORLD, &mpi_status);
            
            for (int j = 0; j < paragraphs_no; ++j) {
                // Primesc lungimea paragrafului
                MPI_Recv(&paragraph_length, 1, MPI_INT, i, i, MPI_COMM_WORLD, &mpi_status);
                
                paragraph = (char*) malloc((paragraph_length + 1) * sizeof(char));
                
                // Primesc textul
                MPI_Recv(paragraph, paragraph_length + 1, MPI_CHAR, i, i, MPI_COMM_WORLD, &mpi_status);

                // Primesc numarul de ordinea al paragrafului
                MPI_Recv(&paragraph_order, 1, MPI_INT, i, i, MPI_COMM_WORLD, &mpi_status);

                /*
                 * Adaug in map la cheie paragraph_order perechea formata din 
                 * genul textului si paragraful in sine
                 */
                paragraphs_map[paragraph_order] = {i, paragraph};

                free(paragraph);
            }    
        }

        // Parcurg cheile din map
        for (int i = 1; i <= paragraphs_map.size(); ++i) {
            // Afisez genul
            if (paragraphs_map[i].first == HORROR) {
                output_stream << horror_genre << "\n";
            } else if (paragraphs_map[i].first == COMEDY) {
                output_stream << comedy_genre << "\n";
            } else if (paragraphs_map[i].first == FANTASY) {
                output_stream << fantasy_genre << "\n";
            } else if (paragraphs_map[i].first == SCIENCE_FICTION) {
                output_stream << science_fiction_genre << "\n";
            }

            // Afisez textul din paragraf
            output_stream << paragraphs_map[i].second;

            if (i < paragraphs_map.size()) {
                output_stream << "\n\n";
            }
        }

        // Inchiderea fisierului de iesire
        output_stream.close();
    } else if (rank == HORROR) {
        MPI_Status status;
        char* paragraph;
        int paragraphs_no; // Numarul paragrafelor
        int paragraph_length; // Lungimea paragrafului
        int paragraph_order; // Ordinea paragrafului
        std::vector<std::string> paragraphs; // Vectorul de paragrafe
        std::vector<int> paragraphs_order; // Vector pentru ordinea paragrafelor

        // Primesc numarul paragrafelor
        MPI_Recv(&paragraphs_no, 1, MPI_INT, master, rank, MPI_COMM_WORLD, &status);

        for (int i = 0; i < paragraphs_no; ++i) {
            // Primesc lungimea paragrafului
            MPI_Recv(&paragraph_length, 1, MPI_INT, master, rank, MPI_COMM_WORLD, &status);
            
            paragraph = (char*) malloc((paragraph_length + 1) * sizeof(char));
            
            // Primesc textul
            MPI_Recv(paragraph, paragraph_length + 1, MPI_CHAR, master, rank, MPI_COMM_WORLD, &status);
            // Adaug paragraful in vectorul de paragrafe
            paragraphs.emplace_back(paragraph);

            // Primesc numarul de ordine al paragrafului
            MPI_Recv(&paragraph_order, 1, MPI_INT, master, rank, MPI_COMM_WORLD, &status);
            
            // Adaug numarul in vector
            paragraphs_order.emplace_back(paragraph_order);

            free(paragraph);
        }

        for (std::string& str : paragraphs) {
            std::vector<char> characters;

            for (char& ch : str) {
                // Adaugam caracterul in vector
                characters.emplace_back(ch);

                // Verificam daca este consoana
                if (is_consonant(ch)) {
                    // Daca este litera mare, o dublam cu o litera mica
                    if (ch >= 'A' && ch <= 'Z') {
                        ch += ('a' - 'A');
                    }

                    characters.emplace_back(ch);
                }
            }

            // Adaugam terminatorul de sir
            characters.emplace_back('\0');
            str = characters.data();
        }

        // Trimitem numarul paragrafelor
        MPI_Send(&paragraphs_no, 1, MPI_INT, master, rank, MPI_COMM_WORLD);

        for (int i = 0; i < paragraphs_no; ++i) {
            int length = paragraphs[i].size();
            char* buff = (char*) malloc((length + 1) * sizeof(char));

            strcpy(buff, paragraphs[i].c_str());

            // Trimit lungimea paragrafului
            MPI_Send(&length, 1, MPI_INT, master, rank, MPI_COMM_WORLD);
            
            // Trimit textul
            MPI_Send(buff, length + 1, MPI_CHAR, master, rank, MPI_COMM_WORLD);
            
            // Trimit numarul de ordine al paragrafului
            MPI_Send(&(paragraphs_order[i]), 1, MPI_INT, master, rank, MPI_COMM_WORLD);

            free(buff);
        } 
    } else if (rank == COMEDY) {
        MPI_Status status;
        char* paragraph;
        int paragraphs_no; // Numarul paragrafelor
        int paragraph_length; // Lungimea paragrafului
        int paragraph_order; // Ordinea paragrafului
        std::vector<std::string> paragraphs; // Vectorul de paragrafe
        std::vector<int> paragraphs_order; // Vector pentru ordinea paragrafelor

        // Primesc numarul paragrafelor
        MPI_Recv(&paragraphs_no, 1, MPI_INT, master, rank, MPI_COMM_WORLD, &status);

        for (int i = 0; i < paragraphs_no; ++i) {
            // Primesc lungimea paragrafului
            MPI_Recv(&paragraph_length, 1, MPI_INT, master, rank, MPI_COMM_WORLD, &status);
            
            paragraph = (char*) malloc((paragraph_length + 1) * sizeof(char));
            
            // Primesc textul
            MPI_Recv(paragraph, paragraph_length + 1, MPI_CHAR, master, rank, MPI_COMM_WORLD, &status);
            
            // Adaug paragraful in vectorul de paragrafe
            paragraphs.emplace_back(paragraph);

            // Primesc numarul de ordine al paragrafului
            MPI_Recv(&paragraph_order, 1, MPI_INT, master, rank, MPI_COMM_WORLD, &status);
            
            // Adaug numarul in vector
            paragraphs_order.emplace_back(paragraph_order);

            free(paragraph);
        }

        for (std::string& str : paragraphs) {
            std::vector<char> characters;
            unsigned int position = 1;

            for (char& ch : str) {
                // Daca am o litera mica pe o pozitie para, o fac majuscula
                if (ch >= 'a' && ch <= 'z' && (position & 1) == 0) {
                    ch -= ('a' - 'A');
                }

                // Daca am spatiu sau '\n' resetez pozitia, altfel o cresc
                if (ch == '\n' || ch == ' ') {
                    position = 1;
                } else {
                    position++;
                }
            }
        }

        // Trimitem numarul paragrafelor
        MPI_Send(&paragraphs_no, 1, MPI_INT, master, rank, MPI_COMM_WORLD);

        for (int i = 0; i < paragraphs_no; ++i) {
            int length = paragraphs[i].size();
            char* buff = (char*) malloc((length + 1) * sizeof(char));

            strcpy(buff, paragraphs[i].c_str());

            // Trimit lungimea paragrafului
            MPI_Send(&length, 1, MPI_INT, master, rank, MPI_COMM_WORLD);
            
            // Trimit textul
            MPI_Send(buff, length + 1, MPI_CHAR, master, rank, MPI_COMM_WORLD);
            
            // Trimit numarul de ordine al paragrafului
            MPI_Send(&(paragraphs_order[i]), 1, MPI_INT, master, rank, MPI_COMM_WORLD);

            free(buff);
        }
    } else if (rank == FANTASY) {
        MPI_Status status;
        char* paragraph;
        int paragraphs_no; // Numarul paragrafelor
        int paragraph_length; // Lungimea paragrafului
        int paragraph_order; // Ordinea paragrafului
        std::vector<std::string> paragraphs; // Vectorul de paragrafe
        std::vector<int> paragraphs_order; // Vector pentru ordinea paragrafelor


        // Primesc numarul paragrafelor
        MPI_Recv(&paragraphs_no, 1, MPI_INT, master, rank, MPI_COMM_WORLD, &status);

        for (int i = 0; i < paragraphs_no; ++i) {
            // Primesc lungimea paragrafului
            MPI_Recv(&paragraph_length, 1, MPI_INT, master, rank, MPI_COMM_WORLD, &status);
            
            paragraph = (char*) malloc((paragraph_length + 1) * sizeof(char));
            
            // Primesc textul
            MPI_Recv(paragraph, paragraph_length + 1, MPI_CHAR, master, rank, MPI_COMM_WORLD, &status);
            
            // Adaug paragraful in vectorul de paragrafe
            paragraphs.emplace_back(paragraph);

            // Primesc numarul de ordine al paragrafului
            MPI_Recv(&paragraph_order, 1, MPI_INT, master, rank, MPI_COMM_WORLD, &status);
            
            // Adaug numarul in vector
            paragraphs_order.emplace_back(paragraph_order);

            free(paragraph);
        }

        for (std::string& str : paragraphs) {
            // Flag ce semnaleaza daca litera trebuie scrisa cu majuscula
            bool capitalise = false;

            for (unsigned int i = 0; i < str.size(); ++i) {
                char& ch = str[i];

                // Daca am spatiu sau '\n' variabila trece pe true
                if (ch == ' ' || ch == '\n') {
                    capitalise = true;
                    continue;
                }

                /*
                 * Daca este prima litera din paragraf sau daca capitalise este 
                 * true, scriem litera cu majuscula
                 */
                if (capitalise || i == 0) {
                    if (ch >= 'a' && ch <= 'z') {
                        ch -= ('a' - 'A');
                    }

                    capitalise = false;
                }
            }
        }

        // Trimitem numarul paragrafelor
        MPI_Send(&paragraphs_no, 1, MPI_INT, master, rank, MPI_COMM_WORLD);

        for (int i = 0; i < paragraphs_no; ++i) {
            int length = paragraphs[i].size();
            char* buff = (char*) malloc((length + 1) * sizeof(char));

            strcpy(buff, paragraphs[i].c_str());

            // Trimit lungimea paragrafului
            MPI_Send(&length, 1, MPI_INT, master, rank, MPI_COMM_WORLD);
            
            // Trimit textul
            MPI_Send(buff, length + 1, MPI_CHAR, master, rank, MPI_COMM_WORLD);
            
            // Trimit numarul de ordine al paragrafului
            MPI_Send(&(paragraphs_order[i]), 1, MPI_INT, master, rank, MPI_COMM_WORLD);

            free(buff);
        }
    } else if (rank == SCIENCE_FICTION) {
        MPI_Status status;
        char* paragraph;
        int paragraphs_no; // Numarul paragrafelor
        int paragraph_length; // Lungimea paragrafului
        int paragraph_order; // Ordinea paragrafului
        std::vector<std::string> paragraphs; // Vectorul de paragrafe
        std::vector<int> paragraphs_order; // Vector pentru ordinea paragrafelor

        // Primesc numarul paragrafelor
        MPI_Recv(&paragraphs_no, 1, MPI_INT, master, rank, MPI_COMM_WORLD, &status);

        for (int i = 0; i < paragraphs_no; ++i) {
            // Primesc lungimea paragrafului
            MPI_Recv(&paragraph_length, 1, MPI_INT, master, rank, MPI_COMM_WORLD, &status);
            
            paragraph = (char*) malloc((paragraph_length + 1) * sizeof(char));
            
            // Primesc textul
            MPI_Recv(paragraph, paragraph_length + 1, MPI_CHAR, master, rank, MPI_COMM_WORLD, &status);

            // Adaug paragraful in vectorul de paragrafe
            paragraphs.emplace_back(paragraph);

            // Primesc numarul de ordine al paragrafului
            MPI_Recv(&paragraph_order, 1, MPI_INT, master, rank, MPI_COMM_WORLD, &status);

            // Adaug numarul in vector
            paragraphs_order.emplace_back(paragraph_order);

            free(paragraph);
        }

        for (std::string& str : paragraphs) {
            int spaces = 0; // Numarul de spatii
            std::string new_string; // Variabila in care retin noul sir
            std::string reverse_str; // Sirul ce va fi inversat
            bool is_reverse = false; // Flag pentru inversarea sirului

            for (char& ch : str) {
                // Daca am ajuns la finalul cuvantului si is_reverse este true
                if ((ch == ' ' || ch == '\n') && is_reverse) {
                    // Inversez sirul
                    reverse(reverse_str.begin(), reverse_str.end());

                    // Adaug sirul auxiliar in new_string
                    new_string += reverse_str;

                    // Adaug ultimul caracter din sirul vechi
                    new_string.push_back(ch);

                    is_reverse = false;
                    reverse_str.clear();
                    continue;
                }

                // Daca flagul este activ, adaug litera in sirul auxiliar
                if (is_reverse) {
                    reverse_str.push_back(ch);
                    continue;
                }

                // Daca caracterul este spatiu, cresc numarul de spatii
                if (ch == ' ') {
                    spaces++;
                }

                // Daca caracterul este '\n', resetez numarul de spatii
                if (ch == '\n') {
                    spaces = 0;
                }

                // Daca numarul de spatii este egal cu 6, is_reverse devine true
                if (spaces == 6) {
                    spaces = 0;
                    is_reverse = true;
                }

                // Adaug in noul string caracterul
                new_string.push_back(ch);
            }

            // Daca flagul is_reverse ramane true, se inverseaza ultimul cuvant
            if (is_reverse) {
                reverse(reverse_str.begin(), reverse_str.end());
                new_string += reverse_str;
            }

            // Variabila str ia valoarea noului sir creat
            str = new_string;
        }

        // Trimitem numarul paragrafelor
        MPI_Send(&paragraphs_no, 1, MPI_INT, master, rank, MPI_COMM_WORLD);

        for (int i = 0; i < paragraphs_no; ++i) {
            int length = paragraphs[i].size();
            char* buff = (char*) malloc((length + 1) * sizeof(char));

            strcpy(buff, paragraphs[i].c_str());

            // Trimit lungimea paragrafului
            MPI_Send(&length, 1, MPI_INT, master, rank, MPI_COMM_WORLD);

            // Trimit textul
            MPI_Send(buff, length + 1, MPI_CHAR, master, rank, MPI_COMM_WORLD);

            // Trimit numarul de ordine al paragrafului
            MPI_Send(&(paragraphs_order[i]), 1, MPI_INT, master, rank, MPI_COMM_WORLD);

            free(buff);
        }
    }

    MPI_Finalize();

    return 0;
}
