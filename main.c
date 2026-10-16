/*
 * SPN-16
 * Substitution-Permutation Network with 16 bits key
 * 
 * Author : François Tjoens
 */

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define LARGEUR_MORCEAU 4
#define NOMBRE_MORCEAU 4
#define TAILLE_BLOC LARGEUR_MORCEAU * NOMBRE_MORCEAU
#define ETAGE 4
#define KEY 0xBEBB

typedef uint16_t bloc;
typedef uint16_t key;
typedef uint8_t word;

const uint8_t SBOX[TAILLE_BLOC] = {
    0xE, 0x4, 0xD, 0x1,
    0x2, 0xF, 0xB, 0x8,
    0x3, 0xA, 0x6, 0xC,
    0x5, 0x9, 0x0, 0x7
};

uint8_t R_SBOX[TAILLE_BLOC];

const uint8_t PBOX[16] = {
    6, 11, 0, 9,
    12, 3, 14, 5,
    2, 15, 4, 13,
    8, 1, 10, 7
};

uint8_t R_PBOX[TAILLE_BLOC];

key round_keys[ETAGE];

bloc apply_sbox(bloc entry);
bloc apply_reverse_sbox(bloc entry);
void construct_reverse_sbox();

bloc apply_pbox(bloc entry);
bloc apply_reverse_pbox(bloc entry);
void construct_reverse_pbox();

void generate_round_keys(key key);

char* cypher(char* msg);
char* decypher(char* msg_encr);

int main(int argc, char *argv[]) {
    if (argc < 4) {
        printf("Usage : %s <4 Digits Hexa Key> <CYPHER|DECYPHER> <Message>\n", argv[0]);
        return 1;
    }
    
    char *k = argv[1];
    char *msg = argv[3];

    // Construction reverse sbox
    construct_reverse_sbox();
    // Construction reverse pbox
    construct_reverse_pbox();
    // Génération des clés d'étage (round keys)
    char *end;
    unsigned long value = strtoul(k, &end, 16);
    generate_round_keys((key)value);


    if (strcasecmp(argv[2], "cypher") == 0) {
        printf("%s\n", cypher(msg));
    } 
    else if (strcasecmp(argv[2], "decypher") == 0) {
        printf("%s\n", decypher(msg));
    } else {
        printf("Usage : %s <4 Digits Hexa Key> <CYPHER|DECYPHER> <Message>\n", argv[0]);
        return 1;
    }

    return 0;
}

bloc apply_sbox(bloc entry_bloc) {
    bloc new_bloc = 0;

    for(uint8_t i = 0; i < NOMBRE_MORCEAU; i++) {
        // décalage droite des bits 
        bloc sub_bloc = entry_bloc >> (4 * i);

        // extraction des bits
        sub_bloc = sub_bloc & 0x000F;

        // application de la sbox
        sub_bloc = SBOX[sub_bloc];

        // décalage gauche des bits
        sub_bloc = sub_bloc << (4 * i);

        // placement des bits dans le bloc de réponse
        new_bloc |= sub_bloc;
    }

    return new_bloc;
}

bloc apply_reverse_sbox(bloc entry_bloc) {
    bloc new_bloc = 0;

    for(uint8_t i = 0; i < NOMBRE_MORCEAU; i++) {
        // décalage droite des bits 
        bloc sub_bloc = entry_bloc >> (4 * i);

        // extraction des bits
        sub_bloc = sub_bloc & 0x000F;

        // application de la sbox
        sub_bloc = R_SBOX[sub_bloc];

        // décalage gauche des bits
        sub_bloc = sub_bloc << (4 * i);

        // placement des bits dans le bloc de réponse
        new_bloc |= sub_bloc;
    }

    return new_bloc;
}

void construct_reverse_sbox() {
    for (uint8_t i = 0; i < TAILLE_BLOC; i++) {
        uint8_t value_sbox = SBOX[i];
        R_SBOX[value_sbox] = i;

        // ou R_SBOX[SBOX[i]] = i; // moins lisible
    }
}

bloc apply_pbox(bloc entry_bloc) {
    bloc new_bloc = 0;

    for (uint8_t i = 0; i < TAILLE_BLOC; i++) {
        // décalage
        bloc sub_bit = entry_bloc >> (i);
        
        // test du bit
        sub_bit = sub_bit & 1; 
        
        // récupération de la position
        uint8_t value = PBOX[i];
        
        // décalage gauche
        sub_bit = sub_bit << value;
        
        // placement du bit dans la réponse
        new_bloc |= sub_bit;
    }

    return new_bloc;
}

bloc apply_reverse_pbox(bloc entry_bloc) {
        bloc new_bloc = 0;

    for (uint8_t i = 0; i < TAILLE_BLOC; i++) {
        // décalage
        bloc sub_bit = entry_bloc >> (i);
        
        // test du bit
        sub_bit = sub_bit & 1; 
        
        // récupération de la position
        uint8_t value = R_PBOX[i];
        
        // décalage gauche
        sub_bit = sub_bit << value;
        
        // placement du bit dans la réponse
        new_bloc |= sub_bit;
    }

    return new_bloc;
}

void construct_reverse_pbox() {
    for (uint8_t i = 0; i < TAILLE_BLOC; i++) {
        uint8_t value_sbox = PBOX[i];
        R_PBOX[value_sbox] = i;

        // ou R_PBOX[PBOX[i]] = i; // moins lisible
    }
}

void generate_round_keys(key master_key) {
    key k = master_key;

    for (uint8_t i = 0; i < ETAGE; i++) {

        k = k >> (3 * i);

        // extraction des mots
        word w1 = k & 0x000F;
        word w2 = (k >> 4) & 0x000F;
        word w3 = (k >> 8) & 0x000F;
        word w4 = (k >> 12) & 0x000F;

        // Diversification de la clé
        /// XOR + Substitution
        word nw1 = w3 ^ w2 ^ w4;
        word nw2 = w1 ^ w4 ^ w3;
        word nw3 = w4 ^ w3 ^ w1;
        word nw4 = w2 ^ SBOX[w1 ^ (i & 0xF)]; 

        /// Melange
        k = 
            (key) nw3 << 12 | 
            (key) nw1 << 8 |    
            (key) nw4 << 4 |
            (key) nw2 ;

        /// Permutation
        k = apply_pbox(k);
        
        // Ecriture de la clé
        round_keys[i] = k;
    }
}

char* cypher(char* msg) {
    size_t msg_len = strlen(msg);
    size_t nb_bloc_in_msg = (msg_len + 1) / 2; // un char = 8 bits, un bloc = 16 bits
    
    uint8_t *msg_encrypted = malloc(nb_bloc_in_msg * 2); // on a x bloc de 16 qu'on stocke par 8

    for (size_t i = 0; i < nb_bloc_in_msg; i++) {
        bloc b = (uint8_t)(msg[2 * i]); // on prend les 8 premiers bits
        b = b << 8; // on les décale pour ajouter les 8 suivants pour former un bloc

        if (2 * i + 1 < msg_len) { // si un second octet
            b |= (uint8_t)msg[2 * i + 1];
        } else { // Padding : ajout de 0x00
            b |= 0x00;
        }
        
        // encrypt
        for (uint8_t j = 0; j < ETAGE; j++) {
            // XOR key
            b ^= round_keys[j];
            // Sub
            b = apply_sbox(b);
            // Perm
            b = apply_pbox(b);
        }

        // Dernier tour (pourrait être plus complexe, ici c'est pour la forme)
        b = apply_sbox(b);

        msg_encrypted[2*i] = (uint8_t)(b >> 8); // prendre les 8 bits de gauche
        msg_encrypted[2*i+1] = (uint8_t)(b & 0xFF); // ne garder que les 8 bits de droite
    }

    char *msg_encr = malloc(nb_bloc_in_msg * 4 + 1);
    
    for (size_t i = 0; i < nb_bloc_in_msg * 2; i++) {
        sprintf(msg_encr + 2 * i, "%02X", (unsigned int)msg_encrypted[i]);
    }
    
    return msg_encr;
}

char* decypher(char* msg_encr) {
    size_t msg_enc_len = strlen(msg_encr);
    size_t nb_bloc_in_msg_enc = msg_enc_len / 4; // 4 parce que c'est de l'hexa

    if (msg_enc_len % 4 != 0) {
        printf("Longueur hexadecimale invalide\n");
        return NULL;
    }

    char *msg_decrypted = malloc(nb_bloc_in_msg_enc * 2 + 1); // + 1 pour le \0

    for (size_t i = 0; i < nb_bloc_in_msg_enc; i++) {
        unsigned int value = 0;
        
        sscanf(msg_encr + 4 * i, "%4X", &value); // à partir de l'adresse msg_encr + 4 * i, lit les prochains format 04X et place le dans bits;
        
        bloc b = (bloc)value;

        // Dernier tour 
        b = apply_reverse_sbox(b);
        
        // decryp
        for (int j = ETAGE-1; j >= 0; j--) {
            // Perm
            b = apply_reverse_pbox(b);
            
            // Sub
            b = apply_reverse_sbox(b);
            
            // XOR key
            b ^= round_keys[j];
        }

        msg_decrypted[2*i] = (char)(uint8_t)(b >> 8); // prendre les 8 bits de gauche
        msg_decrypted[2*i+1] = (char)(uint8_t)(b & 0xFF); // ne garder que les 8 bits de droite
    }

    // Fin de chaîne
    msg_decrypted[nb_bloc_in_msg_enc * 2] = '\0';

    return msg_decrypted;
}