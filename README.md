# Trabajo Final PEV 2 - Hide And Sink
**Alumno:** Agustin Godoy | **Legajo:** [Número] | **Modo:** Prop Hunt | **Versión de Unreal:** UE 5.8.4
---
## Reglas y Mecánica Principal
- **Mecánica:** Hide and Sink es un prop Hunt de 2 a 4 jugadores, en el que los jugadores son pulpos y deben esconderse en el mapa transformandose en props, y un jugador
aleatorio es el cazador, teniendo que buscar y dispararle a los pulpos antes que se termine el tiempo.
- **Cómo se gana:** El cazador debe matar a todos los pulpos, y gana 100 puntos por pulpo que encontró. Los pulpos deben permanecer vivos toda la ronda para ganar 100 puntos,
y ganan 15 puntos más cada vez que el cazador les pasa cerca sin encontrarlos. Se juegan 3 rondas y el ganador es quien tenga mas puntos al final. La idea en caso de continuar el proyecto,
es hacer que en el lobby se pueda configurar la cantidad de rondas, el tiempo de rondas, tiempo para esconderse. 

- **Controles:** 
Pulpos: 
W A S D para moverse, mouse para mover cámara, E para transformarse, Q para volver a pulpo, mantener Click derecho + WASD para rotar el prop convertido. 
Cazador: 
W A S D para moverse, mouse para apuntar, click izquierdo para disparar (3 seg cooldown). 
Ambos: Escape para menú de pausa. 
---
## Ficha Técnica de Red
- **RepNotify:** `RoundPhase`, `RoundWinner`, `RemainingTime`, `AliveOctopusCount` en `HISGameState` -> Como los nombres indican, RoundPhase en el Gamestate actualiza
la fase de ronda entre Waiting, Hiding, Hunting y Ended; RemainingTime actualiza el tiempo restante, AliveOctopusCount actualiza los pulpos vivos que quedan en la ronda y 
RoundWinner actualiza el ganador entre nadie, cazador y pulpos. Cuando se modifican estos valores desde el servidor, se replica a todos los clientes. 
- **Server RPC:** `ServerTryTransform` en `AhideandsinkCharacter` -> El servidor valida que el cliente que lo haya pedido tenga el rol Octopus para hacer la modificacion del DisguiseProp.
- **Client RPC:** `ClientNotifyCaught()` en `hideandsinkCharacter.h` -> el servidor lo llama desde hideandsinkGameMode.cpp en RegisterOctopusCaught(), pero se ejecuta en el cliente propietario de ese personaje.
- **NetMulticast RPC:** `MulticastPlayHarpoonEffects()` en `AHISHunterCharacter` -> VFX Global del arpón, reproducido en el host y en todos los clientes conectados
con un attenuator para que se escuche mas despacio a la distancia.
---
## Cómo Probar el Proyecto
El proyecto ya está preparado para funcionar con el plugin de Steam, se puede probar entre 2 pcs con la misma build, y crear por ese medio.
En caso de no ser por medio de Steam, al crear enviará directo a todos a la partida en vez de pedir unirse.
1. Abrir el mapa: `Content/Maps/HuntLevel1`.
2. En Play (PIE): `Number of Players = 2` (o 4+) y `Net Mode = Play as Listen Server`.
