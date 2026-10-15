/*
 * SPN-16
 * Substitution-Permutation Network with 16 bits key
 * 
 * Author : François Tjoens
 */

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#define LARGEUR_MORCEAU 4
#define NOMBRE_MORCEAU 4
#define TAILLE_BLOC LARGEUR_MORCEAU * NOMBRE_MORCEAU
#define KEY 0b1010111010111011

typedef uint16_t bloc;

const uint8_t SBOX[TAILLE_BLOC] = {
    0xE, 0x4, 0xD, 0x1,
    0x2, 0xF, 0xB, 0x8,
    0x3, 0xA, 0x6, 0xC,
    0x5, 0x9, 0x0, 0x7
};

uint8_t R_SBOX[TAILLE_BLOC];

const uint8_t PBOX[TAILLE_BLOC] = {
    0, 4, 8, 12,
    1, 5, 9, 13,
    2, 6, 10, 14,
    3, 7, 11, 15
};

bloc apply_sbox(bloc entry);
bloc apply_reverse_sbox(bloc entry);
void construct_reverse_sbox();

int main() {
    // Construction reverse sbox
    construct_reverse_sbox();
    // Construction reverse pbox

    // Test SBOX 
    bloc b1 = 0x0004; // 0000 0000 0000 0100
    bloc resp_attendue = 0xEEE2; // 1110 1110 1110 0010 // 0XE 0XE 0XE 0X2

    bloc resp_sbox = apply_sbox(b1);

    printf("### Test SBOX ###\n\n");
    printf("B1: %04X \n", b1);
    printf("Attendu: %04X \n", resp_attendue);
    printf("Recu: %04X \n", resp_sbox);

    
    printf("\n### Test R_SBOX ###\n\n");
    printf("B1: %04X \n", resp_sbox);
    resp_sbox = apply_reverse_sbox(resp_sbox);
    printf("Attendu: %04X \n", b1);
    printf("Recu: %04X \n", resp_sbox);

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