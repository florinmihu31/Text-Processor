    Mihu Florin - 334CC
    Tema 3 - APD

    Logica de functionare:
    La inceput, creez 4 threaduri in procesul master, cate unul pentru fiecare
gen, pentru a citi din fisier. Citesc linie cu linie din fisier, iar daca 
citesc un nume de gen, atunci setez un flag pentru a imi permite sa salvez 
paragraful. Cat timp ma aflu in genul curent, pun liniile intr-un buffer si il 
adaug la final in vectorul de paragrafe. Totodata, tin si evidenta numarului 
paragrafului in fisier. Pentru fiecare gen, trimit numarul total de paragrafe, 
apoi trimit pe rand fiecare paragraf, impreuna cu lungimea sa si numarul sau de 
ordine.
    Fiecare worker primeste datele procesate de threadurile din master si le 
salveaza in vectori (paragrafele si ordinea lor). Apoi, fiecare incepe sa 
proceseze sirurile, in functie de gen.
    Pentru fiecare sir din workerul responsabil pentru horror se construieste 
un vector de caractere, in care se vor adauga literele din sirul vechi, insa 
consoanele vor fi dublate in litera mica.
    Pentru paragrafele comedy, se va parcurge fiecare litera din sir, iar 
literele mici de pe pozitiile pare din cuvant vor deveni majuscule.
    Pentru paragrafele din genul fantasy, se parcurge fiecare litera a sirului, 
iar daca se intalneste caracterul ' ', urmatorul caracter, daca este o litera mica, va fi transformat in litera mare.
    In paragrafele science fiction, se va construi un sir nou, in care se vor 
adauga caracterele din vechiul sir. De asemenea, dupa intalnirea a 6 spatii se 
va construi un sir auxiliar cu urmatorul cuvant, ce va fi introdus inversat in 
noul sir.
    La final, toate procesele worker trimit catre master numarul de paragrafe 
procesate, iar apoi, pentru fiecare, trimit lungimea textului, textul in sine 
si ordinea sa in fisier.
    Masterul salveaza datele primite intr-un unordered map, ce are cheia un 
numar intreg, reprezentand pozitia paragrafului in fisier, iar valoarea o 
pereche formata din genul paragrafului si textul in sine. Acesta primeste 
datele in ordinea workerilor, intrucat fiecare worker trimite pe un tag diferit.
La final, pentru fiecare cheie din map, acesta scrie in fisier intai genul 
textului, apoi paragraful propriu-zis, la finalul fiecaruia adaugandu-se doua 
randuri noi.
    
    Scalabilitate:
    Solutia prezentata scaleaza, datorita citirii din fisier realizata pe 4 
threaduri diferite de catre procesul master.
    Output-ul checkerului este urmatorul:
    ~~~~~~~~~~~~~~~ ESTABLISHING BASE TIME ~~~~~~~~~~~~~~
    Test input1.txt took 0.0020537376403808594 seconds
    Test input2.txt took 0.11673140525817871 seconds
    Test input3.txt took 0.36194634437561035 seconds
    Test input4.txt took 6.405894994735718 seconds
    Test input5.txt took 9.246815919876099 seconds

    ~~~~~~~~~~~~~~~~~~~ RUNNING TESTS ~~~~~~~~~~~~~~~~~~~
    Test input1.txt took 0.7175374031066895 seconds
    Test input2.txt took 0.46766090393066406 seconds
    Test input3.txt took 0.6181623935699463 seconds
    Test input4.txt took 2.370370626449585 seconds
    Test input5.txt took 5.227424383163452 seconds

    In medie, se obtin timpi asemanatori acestei rulari.

    Observatie:
    Uneori checkerul nu da punctele pentru citirea in paralel.
