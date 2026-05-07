# Watchlion

Watchlion è una piccola libreria C per monitorare processi figli tramite un meccanismo di heartbeat.

L’idea è semplice:

1. un processo principale, chiamato watchdog, avvia uno o più programmi;
2. periodicamente il watchdog manda un segnale ai processi figli;
3. ogni processo figlio deve rispondere chiamando `im_alive()`;
4. se un processo non risponde entro il tempo previsto, il watchdog lo considera morto/non responsivo.

---

# Indice

- [Cos'è Watchlion](#cosè-watchlion)
- [A cosa serve](#a-cosa-serve)
- [Come funziona](#come-funziona)
- [Struttura del progetto](#struttura-del-progetto)
- [Compilazione](#compilazione)
- [Installazione](#installazione)
- [Formato del file di configurazione](#formato-del-file-di-configurazione)
- [Uso lato watchdog](#uso-lato-watchdog)
- [Uso lato processo monitorato](#uso-lato-processo-monitorato)
- [API pubblica](#api-pubblica)
- [Esempio completo](#esempio-completo)
- [Come linkare la libreria](#come-linkare-la-libreria)
- [Come funziona internamente](#come-funziona-internamente)
- [Limiti attuali](#limiti-attuali)
- [Best practice](#best-practice)
- [Troubleshooting](#troubleshooting)

---

# Cos'è Watchlion

Watchlion è una libreria pensata per controllare che alcuni processi siano ancora vivi.

Il nome deriva dall’idea di un “leone da guardia” che controlla periodicamente lo stato dei processi.

La libreria è composta da due parti principali:

- un programma watchdog, cioè il supervisore;
- una libreria da includere nei processi monitorati.

Il watchdog avvia i processi indicati nel file di configurazione e poi li controlla periodicamente.

Ogni processo monitorato deve registrare un handler per un segnale, normalmente `SIGUSR1`, e quando riceve il segnale deve chiamare:

```c
im_alive(getpid());
```

In questo modo comunica al watchdog che è ancora vivo.

---

# A cosa serve

Watchlion può essere utile quando hai più processi separati e vuoi verificare che siano ancora reattivi.

Esempi:

* monitorare processi di acquisizione dati;
* monitorare worker separati;
* monitorare processi industriali;
* controllare moduli software eseguiti come programmi indipendenti;
* rilevare processi bloccati;
* fare logging di processi non responsivi.

Attenzione: Watchlion non sostituisce completamente strumenti come `systemd`, `supervisord`, Docker healthcheck o Kubernetes liveness probe.

Watchlion è pensato per un controllo applicativo semplice e custom.

---

# Come funziona

Il flusso generale è questo:

```text
watchdog
   |
   | legge /etc/watchlion.conf
   |
   | avvia i processi con fork + exec
   |
   | crea una shared memory
   |
   | manda SIGUSR1 ai figli
   |
   | attende alcuni secondi
   |
   | controlla quali figli hanno risposto
   |
   | stampa/logga vivi e morti
```

Ogni figlio deve fare:

```text
processo figlio
   |
   | installa handler SIGUSR1
   |
   | riceve SIGUSR1
   |
   | chiama im_alive(getpid())
   |
   | aggiorna lo stato nella shared memory
```

---

# Struttura del progetto

Esempio di struttura consigliata:

```text
watchlion/
├── app/
│   └── main.c
│
├── include/
│   └── watchlion.h
│
├── src/
│   ├── app.c
│   ├── app.h
│   ├── watchlion.c
│   │
│   ├── parser/
│   │   ├── parser.c
│   │   ├── parser.h
│   │   ├── parsed_data.h
│   │   └── id_struct.h
│   │
│   ├── events/
│   │   ├── counter.c
│   │   ├── counter.h
│   │   ├── queue_sig.c
│   │   └── queue_sig.h
│   │
│   └── utils/
│       ├── timer.c
│       ├── timer.h
│       ├── pid_hash_map.cpp
│       └── pid_hash_map.h
│
├── example/
│   └── example_main.c
│
├── Makefile
└── README.md
```

I file importanti per l’utente finale sono:

```text
include/watchlion.h
lib/libwatchlion.a
lib/libwatchlion.so
```

L’utente finale non dovrebbe usare direttamente i file interni in `src/`.

---

# Compilazione

Per compilare tutto:

```bash
make clean
make
```

Questo genera:

```text
bin/watchliond
bin/example_main
lib/libwatchlion.a
lib/libwatchlion.so
```

Per compilare solo la libreria:

```bash
make lib
```

Per compilare solo il watchdog:

```bash
make daemon
```

Per compilare solo l’esempio:

```bash
make example
```

Per pulire:

```bash
make clean
```

---

# Installazione

Per installare la libreria nel sistema:

```bash
sudo make install
```

Di default installa in:

```text
/usr/local/include/watchlion/watchlion.h
/usr/local/lib/libwatchlion.a
/usr/local/lib/libwatchlion.so
```

Puoi cambiare prefisso:

```bash
sudo make install PREFIX=/opt/watchlion
```

Per disinstallare:

```bash
sudo make uninstall
```

---

# Formato del file di configurazione

Il watchdog legge il file:

```text
/etc/watchlion.conf
```

Formato consigliato:

```text
LOG: /tmp/watchlion.log append
EXE: /home/user/prog1 /home/user/prog2 /bin/sleep
TYPE_LOG: signal end_child
```

Ogni riga ha questa forma:

```text
SEZIONE: valore1 valore2 valore3
```

## Sezione `LOG`

Esempio:

```text
LOG: /tmp/watchlion.log append
```

Significato:

```text
/tmp/watchlion.log    file di log
append                modalità di apertura/scrittura
```

## Sezione `EXE`

Esempio:

```text
EXE: /home/user/prog1 /home/user/prog2
```

Contiene la lista degli eseguibili da avviare e monitorare.

Ogni percorso deve essere un eseguibile valido.

Esempio:

```bash
chmod +x /home/user/prog1
chmod +x /home/user/prog2
```

## Sezione `TYPE_LOG`

Esempio:

```text
TYPE_LOG: signal end_child
```

Valori supportati:

```text
signal
end_child
```

---

# Uso lato watchdog

Il watchdog è il programma principale che avvia e controlla i figli.

Per avviarlo:

```bash
./bin/watchliond
```

Oppure, se installato:

```bash
watchliond
```

Prima di avviarlo, assicurati che esista:

```text
/etc/watchlion.conf
```

Esempio:

```bash
sudo nano /etc/watchlion.conf
```

Contenuto esempio:

```text
LOG: /tmp/watchlion.log append
EXE: /home/user/example_main
TYPE_LOG: signal end_child
```

Poi:

```bash
./bin/watchliond
```

Output atteso:

```text
Avvio watchlion
Mando SIGUSR1 a tutti
PID:1234    VIVO
Mando SIGUSR1 a tutti
PID:1234    VIVO
```

Se un processo non risponde:

```text
PID:1234    MORTO
```

---

# Uso lato processo monitorato

Ogni processo monitorato deve includere:

```c
#include "watchlion.h"
```

Poi deve registrare un handler per il segnale usato dal watchdog.

Esempio minimo:

```c
#include <signal.h>
#include <stdio.h>
#include <unistd.h>

#include "watchlion.h"

static void handler_watchlion(int signo, siginfo_t *info, void *context) {
    (void)signo;
    (void)info;
    (void)context;

    im_alive(getpid());
}

int main(void) {
    struct sigaction sa;

    set_watchlion(&sa, handler_watchlion, SIGUSR1, true);

    while (1) {
        pause();
    }

    return 0;
}
```

Quando il watchdog manda `SIGUSR1`, il processo entra nell’handler e chiama:

```c
im_alive(getpid());
```

Questo aggiorna lo stato del processo nella shared memory.

---

# API pubblica

L’header pubblico è:

```c
#include <watchlion/watchlion.h>
```

oppure, se usi include locale:

```c
#include "watchlion.h"
```

---

## `set_watchlion`

```c
void set_watchlion(struct sigaction *sa,
                   void (* const user_handler)(int, siginfo_t *, void *),
                   int signo,
                   bool mod);
```

Registra un handler per un segnale.

Esempio:

```c
struct sigaction sa;
set_watchlion(&sa, handler_watchlion, SIGUSR1, true);
```

Parametri:

```text
sa              puntatore alla struct sigaction
user_handler    funzione da chiamare quando arriva il segnale
signo           segnale da gestire, ad esempio SIGUSR1
mod             se true, rimuove signo dalla signal mask
```

---

## `im_alive`

```c
bool im_alive(pid_t pid_process);
```

Segnala al watchdog che il processo è vivo.

Uso tipico:

```c
im_alive(getpid());
```

Ritorna:

```text
true     heartbeat inviato correttamente
false    errore
```

---

## `set_behave_post_death`

```c
bool set_behave_post_death(void (* const callback_post_death)(void *),
                           void *context,
                           pid_t pid_process);
```

Permette di registrare una callback da eseguire quando un processo viene considerato morto.

Esempio:

```c
void on_death(void *ctx) {
    printf("Processo morto\n");
}

set_behave_post_death(on_death, NULL, getpid());
```

Attenzione: questa parte dipende dalla logica interna del watchdog e va usata con cautela.

---

# Esempio completo

## File monitorato

`worker.c`

```c
#include <signal.h>
#include <stdio.h>
#include <unistd.h>

#include "watchlion.h"

static void handler_watchlion(int signo, siginfo_t *info, void *context) {
    (void)signo;
    (void)info;
    (void)context;

    printf("Heartbeat inviato dal PID %d\n", getpid());
    im_alive(getpid());
}

int main(void) {
    struct sigaction sa;

    set_watchlion(&sa, handler_watchlion, SIGUSR1, true);

    printf("Worker avviato, PID = %d\n", getpid());

    while (1) {
        pause();
    }

    return 0;
}
```

Compilazione:

```bash
gcc worker.c -I/usr/local/include/watchlion -L/usr/local/lib -lwatchlion -pthread -lrt -lstdc++ -o worker
```

Oppure con `g++`:

```bash
g++ worker.c -I/usr/local/include/watchlion -L/usr/local/lib -lwatchlion -pthread -lrt -o worker
```

Config:

```text
LOG: /tmp/watchlion.log append
EXE: /home/user/worker
TYPE_LOG: signal end_child
```

Avvio watchdog:

```bash
./bin/watchliond
```

---

# Come linkare la libreria

## Link statico

```bash
gcc main.c \
    -I/path/to/watchlion/include \
    -L/path/to/watchlion/lib \
    -lwatchlion \
    -pthread \
    -lrt \
    -lstdc++ \
    -o main
```

## Link dinamico

```bash
gcc main.c \
    -I/usr/local/include/watchlion \
    -L/usr/local/lib \
    -lwatchlion \
    -pthread \
    -lrt \
    -lstdc++ \
    -o main
```

Se il programma non trova `libwatchlion.so`, esegui:

```bash
sudo ldconfig
```

Oppure temporaneamente:

```bash
export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH
```

---

# Nota importante su C e C++

Il progetto contiene anche codice C++:

```text
src/utils/pid_hash_map.cpp
```

Per questo motivo, se compili con `gcc`, devi linkare anche:

```bash
-lstdc++
```

Esempio:

```bash
gcc main.c -lwatchlion -lstdc++ -pthread -lrt -o main
```

In alternativa puoi usare direttamente `g++`:

```bash
g++ main.c -lwatchlion -pthread -lrt -o main
```

---

# Come funziona internamente

Watchlion usa:

```text
fork()
exec()
signals
shared memory POSIX
mmap()
pthread
```

## Avvio dei figli

Il watchdog legge la lista degli eseguibili nel file config.

Poi per ogni eseguibile fa:

```c
fork();
exec();
```

Quindi ogni programma diventa un processo separato.

## Shared memory

Il watchdog crea una shared memory POSIX:

```c
shm_open()
ftruncate()
mmap()
```

La shared memory contiene uno stato per ogni processo.

Esempio concettuale:

```c
struct ProcessState {
    int alive;
    void (*callback_pd)(void *);
    void *context;
};
```

Quando un figlio chiama:

```c
im_alive(getpid());
```

viene settato:

```c
alive = 1;
```

Il watchdog poi legge questo campo.

Se è `1`, il processo è vivo.

Se è `0`, il processo non ha risposto.

Dopo ogni controllo, il watchdog rimette `alive = 0`.

## Perché serve `attach_shm`

Il watchdog crea la shared memory.

Il figlio però viene avviato con `exec`.

Dopo `exec`, il processo perde la vecchia mappa virtuale.

Quindi il figlio deve aprire e rimappare la shared memory.

Per questo la libreria chiama internamente:

```c
attach_shm()
```

---

# Limiti attuali

Questa libreria è funzionante come prototipo, ma ha alcuni limiti importanti.

## 1. Il figlio può mentire

Il figlio può sempre chiamare:

```c
im_alive(getpid());
```

anche se internamente è in stato logico errato.

Watchlion verifica che il processo risponda, non che il programma sia davvero corretto.

## 2. Uso di variabili d’ambiente

Attualmente alcune informazioni vengono passate ai figli tramite variabili d’ambiente:

```text
WATCHLION_SHM_NAME
WATCHLION_INDEX
WATCHLION_COUNT
```

Questo è comodo, ma non è robustissimo.

Un utente potrebbe modificarle manualmente.

## 3. Shared memory scrivibile

Il processo figlio scrive direttamente nella shared memory.

Questo significa che un processo malevolo o scritto male potrebbe corrompere lo stato.

## 4. Non è una sandbox

Watchlion non impedisce al processo di fare danni.

Non limita:

```text
file system
rete
CPU
RAM
permessi utente
```

Per quello servono altri strumenti:

```text
systemd sandboxing
containers
seccomp
AppArmor
SELinux
chroot
namespaces
```

## 5. Callback in shared memory

Salvare puntatori a funzione o puntatori generici in shared memory è fragile.

Un puntatore ha senso solo nello spazio virtuale del processo che lo ha creato.

Per una versione robusta, questa parte andrebbe riprogettata.

---

# Best practice

## Usare percorsi assoluti

Nel file config usa sempre percorsi assoluti:

```text
EXE: /home/user/my_worker
```

Evita:

```text
EXE: ./my_worker
```

## Controllare i permessi

Ogni programma deve essere eseguibile:

```bash
chmod +x /home/user/my_worker
```

## Non fare lavoro pesante nel signal handler

Dentro un signal handler bisognerebbe fare il minimo indispensabile.

Uso accettabile:

```c
im_alive(getpid());
```

Meglio evitare:

```c
printf()
malloc()
free()
fopen()
```

Nota: questa libreria, come prototipo, non è ancora perfetta dal punto di vista async-signal-safe.

Per una versione production, sarebbe meglio usare pipe/socket/eventfd invece di fare logica complessa nel signal handler.

## Non modificare le variabili d’ambiente Watchlion

Non modificare manualmente:

```text
WATCHLION_SHM_NAME
WATCHLION_INDEX
WATCHLION_COUNT
```

Sono usate internamente dalla libreria.

## Non includere header privati

L’utente finale deve includere solo:

```c
#include <watchlion/watchlion.h>
```

Non deve includere:

```c
#include "app.h"
#include "parser.h"
#include "pid_hash_map.h"
```

---

# Troubleshooting

## Errore: `watchlion.h: No such file or directory`

Soluzione:

```bash
gcc main.c -I/usr/local/include/watchlion ...
```

Oppure:

```c
#include <watchlion/watchlion.h>
```

---

## Errore: `undefined reference to im_alive`

La libreria non è linkata.

Aggiungi:

```bash
-lwatchlion
```

Esempio:

```bash
gcc main.c -L/usr/local/lib -lwatchlion -pthread -lrt -lstdc++ -o main
```

---

## Errore: simboli C++ mancanti

Se vedi errori su simboli C++, aggiungi:

```bash
-lstdc++
```

Oppure compila con:

```bash
g++
```

---

## Errore: `cannot open shared object file`

Il sistema non trova `libwatchlion.so`.

Soluzione temporanea:

```bash
export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH
```

Soluzione stabile:

```bash
sudo ldconfig
```

---

## Il processo risulta sempre morto

Controlla che nel processo figlio ci sia:

```c
set_watchlion(&sa, handler_watchlion, SIGUSR1, true);
```

e che l’handler chiami:

```c
im_alive(getpid());
```

Controlla anche che il watchdog stia mandando `SIGUSR1`.

---

## Il watchdog non trova il file config

Il file deve trovarsi qui:

```text
/etc/watchlion.conf
```

Crealo:

```bash
sudo nano /etc/watchlion.conf
```

---

## Il figlio non parte

Controlla:

```bash
ls -l /path/to/programma
```

Deve avere permessi di esecuzione:

```bash
chmod +x /path/to/programma
```

Controlla anche che il path nel config sia assoluto e corretto.

---

# Esempio rapido finale

## 1. Crea worker

```c
#include <signal.h>
#include <unistd.h>
#include "watchlion.h"

static void handler(int signo, siginfo_t *info, void *context) {
    (void)signo;
    (void)info;
    (void)context;

    im_alive(getpid());
}

int main(void) {
    struct sigaction sa;
    set_watchlion(&sa, handler, SIGUSR1, true);

    while (1) {
        pause();
    }

    return 0;
}
```

## 2. Compila worker

```bash
gcc worker.c -I/usr/local/include/watchlion -L/usr/local/lib -lwatchlion -pthread -lrt -lstdc++ -o worker
```

## 3. Crea config

```text
LOG: /tmp/watchlion.log append
EXE: /home/user/worker
TYPE_LOG: signal end_child
```

Salvalo in:

```text
/etc/watchlion.conf
```

## 4. Avvia watchdog

```bash
./bin/watchliond
```

Output atteso:

```text
Avvio watchlion
Mando SIGUSR1 a tutti
PID:1234    VIVO
```

---

# Stato del progetto

Watchlion è attualmente una libreria/prototipo per monitoring tramite segnali e shared memory.

È utile per studio, test e prototipi.

Per un uso realmente production sarebbe consigliato evolvere l’architettura verso:

```text
figlio -> pipe/socket/eventfd -> watchdog
```

invece di far scrivere direttamente il figlio nella shared memory.