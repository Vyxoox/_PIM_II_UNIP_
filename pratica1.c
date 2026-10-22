#include <stdio.h>
#include <locale.h>

int main(){
    setlocale(LC_ALL, "portuguese-brazilian");
    int n = 13;
    int user_input = 0;

    while(user_input != n){
        printf("Escolha um número e tente adivinhar qual o secreto: \n");
        scanf("%d", &user_input);
        if (user_input == n){
            printf("Parabéns! Acertou\n");
            break;
        }else if(user_input <n){
            printf("O seu número é menor que o número secreto");
        }else {
            printf("O seu número é maior que o número secreto");
        }
    }
}