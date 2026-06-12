# Projeto-Backlog-Manager

O Backlog Manager e uma aplicacao de terminal (CLI) desenvolvida em linguagem C com o
objetivo de funcionar como um organizador pessoal de biblioteca de jogos digitais. O usuario
podera cadastrar titulos, atribuir avaliacoes, acompanhar o status de cada jogo e navegar por
seu catalogo de forma visual e fluida, simulando a experiencia de plataformas de streaming.
O grande diferencial da aplicacao e sua interface de navegacao baseada em carrosseis,
implementada sobre Listas Circulares, proporcionando uma experiencia de browsing semelhante
a da Netflix ou Steam. Toda a logica e construida sob rigorosas restricoes tecnicas que exigem
o uso de alocacao dinamica de memoria e estruturas encadeadas por ponteiros.
1.1 Objetivos
• Fornecer uma ferramenta CLI completa para gerenciamento de biblioteca pessoal de
jogos.
• Demonstrar o uso correto e eficiente das principais estruturas de dados lineares.
• Garantir persistencia de dados entre sessoes via arquivos binarios.
• Implementar navegacao visual por carrosseis usando listas circulares.
• Assegurar a ausencia de vazamentos de memoria (memory leaks) e falhas de
segmentacao.
1.2 Restricoes Tecnicas Obrigatorias
ATENCAO: E expressamente PROIBIDO o uso de vetores (arrays) para armazenar colecoes
de dados (ex.: struct Jogo catalogo[100];). Todas as colecoes devem ser implementadas com
malloc/free e estruturas encadeadas. Vetores sao permitidos APENAS para cadeias de
caracteres (strings), como o nome do jogo (char nome[50]).
