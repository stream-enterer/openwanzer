## Source code structure description:
    .
    ├── css                        - style sheets used for ui elements
    │   ├── fonts.css              - fonts used in game
    │   ├── ui-combat-info.css     - animations used during combat
    │   ├── ui-equipment.css       - equipment window style and positioning
    │   ├── ui-message.css         - message windows style and positioning
    │   ├── ui-startmenu.css       - main menu shown when game starts
    │   ├── ui-unit-info.css       - unit information dialog
    │   └── ui.css                 - main window, main menu, generic dom elements
    ├── js
    │   ├── ai.js                  - AI engine
    │   ├── animation.js           - draws a series of sprites in a time interval
    │   ├── dom.js                 - generic function that deal with DOM
    │   ├── eventhandler.js        - event driven function (unused atm)
    │   ├── game.js                - game manager
    │   ├── gamerules.js           - attack, move, resupply, reinforce, distance rules
    │   ├── gamestate.js           - save/load game state to HTML5 local storage
    │   ├── map.js                 - hex, map and player objects
    │   ├── maploader.js           - loads scenarios and maps from a xml file
    │   ├── prototypes.js          - generic definitions
    │   ├── render.js              - canvas rendering functions
    │   ├── sound.js               - generic unit sounds functions
    │   ├── style.js               - canvas style for render.js
    │   ├── unit.js                - unit and transport objects
    │   └── ui.js                  - handle mouse, builds/updates UI windows
    ├── resources
    │   ├── animations             - images with 1 row of sprites for animations
    │   ├── campaigns              - campaigns and campaignlist.js index converted with tools/campaign/campaign-convert.py
    │   ├── equipment              - contains units equipment/properties
    │   ├── fonts                  - fonts used in OpenPanzer
    │   ├── maps                   - big images for the map background
    │   ├── scenarios              - scenarios and scenariolist.js index converted with tools/map/mapconvert.py
    │   ├── sounds                 - sounds used for units
    │   ├── ui
    │   │   ├── buttons            - generic buttons (ok/close)
    │   │   ├── cursors            - mouse cursors used in game
    │   │   ├── dialogs
    │   │   │   ├── equipment      - images for equipment buttons/dialog
    │   │   │   ├── startmenu      - images for start menu dialog
    │   │   │   ├── ui-message     - images for message dialog
    │   │   │   ├── unit-context   - images for buttons that pop up on unit selections
    │   │   │   └── unit-info      - images for unit info stats
    │   │   ├── flags              - small (for cities) and big (for unit info) flags
    │   │   ├── indicators         - small indicators that are drawn on the map (unit has fired)
    │   │   ├── menu               - images used on main window text bar or main menu
    │   │   ├── splash             - splash images used as loading screen on android/ios
    │   │   ├── page               - images used outside main window
    │   └── units                  - 1x9 sprites with unit orientations for each unit
    ├── tools
    │    ├── campaign              - converts campaigns from PG2 .cam format
    │    ├── equipment             - converts PG2Suite exported equipment to js equipment
    │    ├── icons                 - converts SHPTool exported bmp to transparent png
    │    └── map                   - converts SCN,MAP,TXT files to OpenPanzer XML
    │
    └── index.html                 - html file with basic DOM structure for openpanzer

## Development notes:
-  Respect latest W3.org HTML5, CSS3 specs and browser specific specs are kept to a minimum -> null.
-  Base supported javascript engine is V8 open source engine. Code should be kept to basic features of ECMAScript 5.
-  Don't use any extra javascript libraries (like jquery, node etc), and should be kept like this.
-  Styling DOM elements should be done in their CSS files. Adding style for static elements in the code shouldn't exist.
-  All positioning of DOM elements should be done in CSS files. Bottom line: if something can be done in a css file then there it should go not in the javascript code.
-  Code additions should follow the coding style used so far (for object creation etc).

Main testing is done on Google Chrome and Firefox under Linux.

Running the code locally in Google Chrome requires --allow-file-access-from-files option when starting Google Chrome.
