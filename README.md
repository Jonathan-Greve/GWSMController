# GWSMController - Guild Wars Shared Memory Controller

GWSMController is a powerful DirectX 11 application developed based on the [GuildWarsMapBrowser](https://github.com/Jonathan-Greve/GuildWarsMapBrowser). It uses data from the [GWSM (Guild Wars Shared Memory)](https://github.com/Jonathan-Greve/GWSM) library to render in-game actors/agents on 3D maps. It work with multiple active Guild Wars processes, just remember to inject [GWSM](https://github.com/Jonathan-Greve/GWSM) into all of them.

![GWSMController Preview](images/preview0.png)

This application provides an enriched, interactive, and visually appealing way of viewing and exploring Guild Wars environments in three dimensions.

## Core Features of GWSMController

- **Data Integration from GWSM:** GWSMController leverages the GWSM library for accessing shared data from the game. This enables the application to render in-game elements accurately in a 3D environment.
- **3D Map Rendering:** By building upon the work done in the GuildWarsMapBrowser, GWSMController allows users to view detailed 3D maps pulled from the game, providing a unique perspective on the game's environments.
- **Real-time Actor/Agent Rendering:** GWSMController can display in-game actors/agents on the 3D maps in real-time, providing an immersive and dynamic map viewing experience.

You can watch this brief [demo video](https://www.youtube.com/watch?v=iyrrJm9KUsM) to see GWSMController in action.

## How to Use GWSMController
See the current release. In short you must inject GWSM into one (or multiple) running Guild Wars processes. Then launch GWSMController (you can also launch it before injecting, it doesn't matter which order).

## Contributing

While I appreciate pull requests, please note that I am not actively developing this library and may not merge them. However, feel free to fork the repository and adapt it to your needs.

## Acknowledgments

This application is built upon the foundational work done in [GuildWarsMapBrowser](https://github.com/Jonathan-Greve/GuildWarsMapBrowser) and [GWSM](https://github.com/Jonathan-Greve/GWSM). Many thanks to the authors and contributors of these projects for their significant contributions to the Guild Wars modding community.
