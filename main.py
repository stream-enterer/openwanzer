"""
Panzer General 2 Prototype - Main Game
Python Arcade Implementation
"""

import arcade
import arcade.gui
from arcade.camera import Camera2D
from arcade.types import LBWH, Rect
import math
from constants import *
from game_state import GameState
from ai import SimpleAI

class PanzerGame(arcade.Window):
    """Main game window"""
    
    def __init__(self):
        super().__init__(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_TITLE, resizable=True, vsync=False, update_rate=1/120)

        arcade.set_background_color(arcade.color.BLACK)

        # Game state
        self.game_state = None
        self.ai = None

        # UI state
        self.selected_unit = None
        self.reachable_hexes = {}
        self.attackable_hexes = []
        self.highlighted_hex = None

        # Camera for scrollable game viewport
        self.game_camera = Camera2D()
        self.camera_x = 0  # Camera position for panning
        self.camera_y = 0
        self.camera_speed = 10  # Pixels per frame when panning

        # Middle mouse drag state
        self.middle_mouse_pressed = False
        self.last_mouse_x = 0
        self.last_mouse_y = 0

        # UI Manager
        self.ui_manager = arcade.gui.UIManager()

        # Layout proportions (70% game, 30% HUD)
        self.game_panel_ratio = 0.7
        self.hud_panel_ratio = 0.3

        # Calculated panel widths (updated on resize)
        self.game_panel_width = int(self.width * self.game_panel_ratio)
        self.hud_panel_width = int(self.width * self.hud_panel_ratio)

        # HUD widgets (references for updating)
        self.turn_label = None
        self.player_label = None
        self.units_label = None
        self.selected_unit_label = None
        self.ai_thinking_label = None
        self.hud_box = None
        self.message_box = None

        # Message log
        self.messages = []
        self.max_messages = 100

        # AI processing
        self.ai_thinking = False
        self.ai_delay = 0
        
    def setup(self):
        """Set up the game"""
        self.game_state = GameState()
        self.ai = SimpleAI(self.game_state, self.game_state.players[1])
        self.selected_unit = None
        self.reachable_hexes = {}
        self.attackable_hexes = []

        # Center camera on the hex grid
        # Calculate the center of the hex map in world coordinates
        map_center_col = MAP_COLS / 2
        map_center_row = MAP_ROWS / 2

        # Use the same offset values as in hex_map.py get_pixel_position()
        offset_x = 100
        offset_y = 50

        # Calculate world position of map center using hex pixel conversion
        map_center_world_x = offset_x + HEX_SIZE * 1.5 * map_center_col
        map_center_world_y = offset_y + HEX_HEIGHT * (map_center_row + 0.5 * (int(map_center_col) % 2))

        # Camera position is what world position appears at screen (0,0)
        # To center the map, we want map_center to appear at viewport center
        # viewport_center = (game_panel_width/2, height/2)
        # So: map_center_world = viewport_center + camera_position
        # Therefore: camera_position = map_center_world - viewport_center
        self.camera_x = map_center_world_x - (self.game_panel_width / 2)
        self.camera_y = map_center_world_y - (self.height / 2)

        # Setup GUI
        self.setup_gui()

        # Add initial message
        if not self.messages:  # Only add if messages is empty (new game)
            self.add_message("=== Game started ===")
            self.add_message(f"=== {self.game_state.get_current_player().get_name()}'s turn ===")
            self.add_message("Deploy your units and capture objectives!")

    def setup_gui(self):
        """Setup the GUI layout with horizontal UIBoxLayout for 70/30 split"""
        self.ui_manager.clear()

        # Create a horizontal box layout for the main 70/30 split
        main_layout = arcade.gui.UIBoxLayout(
            vertical=False,  # Horizontal layout
            space_between=0
        )

        # Create game panel (left side) - transparent spacer to reserve space
        game_panel = arcade.gui.UISpace(
            color=(0, 0, 0, 0),  # Transparent
            size_hint=(self.game_panel_ratio, 1),
            size_hint_min=(int(self.width * self.game_panel_ratio), self.height)
        )

        # Create HUD panel container with dark background
        hud_container = arcade.gui.UIAnchorLayout(
            size_hint=(self.hud_panel_ratio, 1),
            size_hint_min=(int(self.width * self.hud_panel_ratio), self.height)
        )

        # Dark background for HUD
        hud_background = arcade.gui.UISpace(
            color=(20, 20, 20, 255),
            size_hint=(1, 1)
        )
        hud_container.add(hud_background, anchor_x="left", anchor_y="top")

        # Create a vertical box layout for HUD content (top 75% and bottom 25%)
        hud_vertical_layout = arcade.gui.UIBoxLayout(
            vertical=True,
            space_between=0
        )

        # Top section (75% of HUD) - contains game info
        top_section = arcade.gui.UIBoxLayout(
            vertical=True,
            space_between=10,
            size_hint=(1, 0.75)
        )

        # Create HUD labels
        self.turn_label = arcade.gui.UILabel(
            text="Turn: 1/20",
            font_size=14,
            bold=True,
            text_color=arcade.color.WHITE,
            width=self.hud_panel_width - 40
        )

        self.player_label = arcade.gui.UILabel(
            text="Current: Axis",
            font_size=14,
            bold=True,
            text_color=arcade.color.RED,
            width=self.hud_panel_width - 40
        )

        self.units_label = arcade.gui.UILabel(
            text="Units: 0",
            font_size=12,
            text_color=arcade.color.WHITE,
            width=self.hud_panel_width - 40
        )

        # Spacer
        spacer1 = arcade.gui.UISpace(height=20, color=(0, 0, 0, 0))

        self.selected_unit_label = arcade.gui.UILabel(
            text="",
            font_size=10,
            text_color=arcade.color.WHITE,
            width=self.hud_panel_width - 40,
            multiline=True
        )

        # Spacer
        spacer2 = arcade.gui.UISpace(height=40, color=(0, 0, 0, 0))

        # Controls label
        controls_label = arcade.gui.UILabel(
            text="Controls:\nLeft Click: Select/Move\nRight Click: Attack\nMiddle Click+Drag: Pan\nSPACE: End turn\nESC: Deselect\nArrows/WASD: Pan camera",
            font_size=10,
            text_color=arcade.color.WHITE,
            width=self.hud_panel_width - 40,
            multiline=True
        )

        # AI thinking indicator
        self.ai_thinking_label = arcade.gui.UILabel(
            text="",
            font_size=14,
            bold=True,
            text_color=arcade.color.YELLOW,
            width=self.hud_panel_width - 40
        )

        # Add widgets to top section
        top_section.add(self.turn_label)
        top_section.add(self.player_label)
        top_section.add(self.units_label)
        top_section.add(spacer1)
        top_section.add(self.selected_unit_label)
        top_section.add(spacer2)
        top_section.add(controls_label)
        top_section.add(self.ai_thinking_label)

        # Wrap top section in a padding container
        top_section_wrapper = arcade.gui.UIAnchorLayout(size_hint=(1, 0.75))
        top_section_wrapper.add(
            top_section,
            anchor_x="left",
            anchor_y="top",
            align_x=20,
            align_y=-20
        )

        # Bottom section (25% of HUD) - message box
        message_box_height = int(self.height * 0.25)

        # Create message box with border
        message_box_container = arcade.gui.UIAnchorLayout(
            size_hint=(1, 0.25),
            size_hint_min=(self.hud_panel_width, message_box_height)
        )

        # Message box background (dark gray with border)
        message_bg = arcade.gui.UISpace(
            color=(30, 30, 30, 255),
            size_hint=(1, 1)
        )
        message_box_container.add(message_bg, anchor_x="left", anchor_y="bottom")

        # Create text area for messages
        self.message_box = arcade.gui.UITextArea(
            text="=== Game Messages ===\n",
            width=self.hud_panel_width - 44,
            height=message_box_height - 44,
            font_size=9,
            text_color=arcade.color.WHITE
        )

        # Create a bordered wrapper for the message box
        message_border = arcade.gui.UIBorder(
            child=self.message_box,
            border_width=1,
            border_color=arcade.color.WHITE
        )

        # Position message box with padding
        message_box_container.add(
            message_border,
            anchor_x="left",
            anchor_y="bottom",
            align_x=20,
            align_y=20
        )

        # Add sections to vertical layout
        hud_vertical_layout.add(top_section_wrapper)
        hud_vertical_layout.add(message_box_container)

        # Add vertical layout to HUD container
        hud_container.add(
            hud_vertical_layout,
            anchor_x="left",
            anchor_y="top"
        )

        # Add both panels to main layout
        main_layout.add(game_panel)
        main_layout.add(hud_container)

        # Add main layout to UI manager
        self.ui_manager.add(main_layout)
        self.ui_manager.enable()

        # Initial update of HUD
        self.update_hud()

    def add_message(self, message, color=arcade.color.WHITE):
        """Add a message to the message log"""
        # Add timestamp or turn number
        turn_str = f"[T{self.game_state.turn}]" if self.game_state else "[--]"
        formatted_message = f"{turn_str} {message}"

        self.messages.append(formatted_message)

        # Keep only the last max_messages
        if len(self.messages) > self.max_messages:
            self.messages = self.messages[-self.max_messages:]

        # Update message box text
        if self.message_box:
            message_text = "=== Game Messages ===\n" + "\n".join(self.messages)
            self.message_box.text = message_text

    def on_resize(self, width, height):
        """Handle window resize"""
        super().on_resize(width, height)

        # Update panel widths
        self.game_panel_width = int(width * self.game_panel_ratio)
        self.hud_panel_width = int(width * self.hud_panel_ratio)

        # Rebuild GUI with new dimensions
        if self.game_state:
            self.setup_gui()

    def update_hud(self):
        """Update HUD labels with current game state"""
        if not self.game_state:
            return

        # Update turn
        self.turn_label.text = f"Turn: {self.game_state.turn}/{self.game_state.max_turns}"

        # Update player
        player = self.game_state.get_current_player()
        self.player_label.text = f"Current: {player.get_name()}"
        self.player_label.text_color = player.get_color()

        # Update units
        self.units_label.text = f"Units: {len(player.get_active_units())}"

        # Update selected unit info
        if self.selected_unit:
            self.selected_unit_label.text = f"Selected Unit:\n{self.selected_unit.get_info_string()}"
        else:
            self.selected_unit_label.text = ""

        # Update AI thinking indicator
        if self.ai_thinking:
            self.ai_thinking_label.text = "AI Thinking..."
        else:
            self.ai_thinking_label.text = ""
        
    def on_draw(self):
        """Render the game"""
        self.clear()

        # Set viewport to game panel area only (left 70%)
        # LBWH = left, bottom, width, height
        self.game_camera.viewport = LBWH(0, 0, self.game_panel_width, self.height)

        # Update projection to match viewport for 1:1 pixel ratio
        # This prevents squashing/stretching when window is resized
        # Use from_kwargs() factory method to create Rect with bounds only
        self.game_camera.projection = Rect.from_kwargs(
            left=0,
            right=self.game_panel_width,
            bottom=0,
            top=self.height
        )

        # Position camera for scrollable viewport
        self.game_camera.position = (self.camera_x, self.camera_y)

        # Use the camera
        self.game_camera.use()

        # Draw map
        self.draw_map()

        # Draw units
        self.draw_units()

        # Reset to default camera for UI rendering (full screen viewport)
        self.default_camera.use()

        # Draw UI manager (HUD panels and widgets)
        self.ui_manager.draw()

        # Draw game over screen
        if self.game_state.game_over:
            self.draw_game_over()
    
    def draw_map(self):
        """Draw the hex map"""
        for row in self.game_state.hex_map.hexes:
            for hex_tile in row:
                x, y = hex_tile.get_pixel_position()

                # Get color based on terrain
                color = TERRAIN_COLORS[hex_tile.terrain]

                # Draw hex
                self.draw_hex(x, y, color)

                # Draw hex border
                self.draw_hex_outline(x, y, COLOR_HEX_BORDER, 2)

                # Highlight if selected, reachable, or attackable
                if hex_tile == self.highlighted_hex:
                    self.draw_hex_outline(x, y, (255, 255, 255), 3)
                elif hex_tile in self.reachable_hexes:
                    self.draw_hex(x, y, COLOR_MOVEABLE, filled=False)
                elif hex_tile in self.attackable_hexes:
                    self.draw_hex(x, y, COLOR_ATTACKABLE, filled=False)

                # Draw objective marker
                if hex_tile.is_objective:
                    marker_color = arcade.color.WHITE
                    if hex_tile.owner is not None:
                        marker_color = SIDE_COLORS[hex_tile.owner]
                    arcade.draw_circle_filled(x, y, 8, marker_color)
                    arcade.draw_circle_outline(x, y, 8, arcade.color.BLACK, 2)
    
    def draw_hex(self, x, y, color, filled=True):
        """Draw a hexagon"""
        points = self.get_hex_points(x, y)
        
        if filled:
            arcade.draw_polygon_filled(points, color)
        else:
            arcade.draw_polygon_outline(points, color, 3)
    
    def draw_hex_outline(self, x, y, color, line_width):
        """Draw hex outline"""
        points = self.get_hex_points(x, y)
        arcade.draw_polygon_outline(points, color, line_width)
    
    def get_hex_points(self, x, y):
        """Get the corner points of a hex"""
        points = []
        for i in range(6):
            angle = math.pi / 3 * i
            point_x = x + HEX_SIZE * math.cos(angle)
            point_y = y + HEX_SIZE * math.sin(angle)
            points.append((point_x, point_y))
        return points
    
    def draw_units(self):
        """Draw all units"""
        for player in self.game_state.players:
            for unit in player.get_active_units():
                if unit.hex:
                    x, y = unit.hex.get_pixel_position()

                    # Unit color based on side
                    color = SIDE_COLORS[unit.owner]

                    # Draw unit as rectangle
                    size = 20
                    # lrbt = left, right, bottom, top (bottom < top)
                    arcade.draw_lrbt_rectangle_filled(
                        x - size/2, x + size/2, y - size/2, y + size/2, color
                    )
                    arcade.draw_lrbt_rectangle_outline(
                        x - size/2, x + size/2, y - size/2, y + size/2,
                        arcade.color.BLACK, 2
                    )

                    # Draw unit type indicator
                    unit_text = self.get_unit_symbol(unit)
                    arcade.draw_text(unit_text, x, y, arcade.color.WHITE,
                                   12, anchor_x="center", anchor_y="center",
                                   bold=True)

                    # Draw strength bar
                    bar_width = 20
                    bar_height = 3
                    bar_x = x
                    bar_y = y - size / 2 - 8

                    # Background (red) - lrbt format
                    arcade.draw_lrbt_rectangle_filled(
                        bar_x - bar_width/2, bar_x + bar_width/2,
                        bar_y - bar_height/2, bar_y + bar_height/2,
                        arcade.color.RED
                    )
                    # Health (green) - lrbt format
                    health_width = bar_width * (unit.strength / 10.0)
                    arcade.draw_lrbt_rectangle_filled(
                        bar_x - bar_width/2, bar_x - bar_width/2 + health_width,
                        bar_y - bar_height/2, bar_y + bar_height/2,
                        arcade.color.GREEN
                    )

                    # Highlight selected unit
                    if unit == self.selected_unit:
                        arcade.draw_lrbt_rectangle_outline(
                            x - (size + 4)/2, x + (size + 4)/2,
                            y - (size + 4)/2, y + (size + 4)/2,
                            COLOR_SELECTED, 3
                        )
    
    def get_unit_symbol(self, unit):
        """Get text symbol for unit type"""
        unit_class = unit.get_class()
        symbols = {
            UnitClass.INFANTRY: "I",
            UnitClass.TANK: "T",
            UnitClass.RECON: "R",
            UnitClass.ANTI_TANK: "A",
            UnitClass.ARTILLERY: "X",
            UnitClass.FIGHTER: "F",
            UnitClass.BOMBER: "B"
        }
        return symbols.get(unit_class, "?")

    def draw_game_over(self):
        """Draw game over screen"""
        # Semi-transparent overlay - lrbt format (left, right, bottom, top)
        arcade.draw_lrbt_rectangle_filled(
            0, self.width,
            0, self.height,
            (0, 0, 0, 200)
        )

        # Game over text
        arcade.draw_text("GAME OVER", self.width / 2, self.height / 2 + 50,
                        arcade.color.WHITE, 48, bold=True,
                        anchor_x="center", anchor_y="center")

        # Winner
        winner_text = f"{SIDE_NAMES[self.game_state.winner]} Wins!"
        winner_color = SIDE_COLORS[self.game_state.winner]
        arcade.draw_text(winner_text, self.width / 2, self.height / 2,
                        winner_color, 36, bold=True,
                        anchor_x="center", anchor_y="center")

        # Restart instruction
        arcade.draw_text("Press R to Restart", self.width / 2,
                        self.height / 2 - 50,
                        arcade.color.WHITE, 20,
                        anchor_x="center", anchor_y="center")
    
    def on_mouse_press(self, x, y, button, modifiers):
        """Handle mouse clicks"""
        # Handle middle mouse button for camera panning (works in game panel)
        if button == arcade.MOUSE_BUTTON_MIDDLE:
            if x < self.game_panel_width:
                self.middle_mouse_pressed = True
                self.last_mouse_x = x
                self.last_mouse_y = y
            return

        if self.game_state.game_over:
            return

        # Only allow clicks in game panel (not in HUD)
        if x >= self.game_panel_width:
            return

        # Only allow human player to click
        if self.game_state.current_player.player_type != PlayerType.HUMAN:
            return

        # Find clicked hex
        clicked_hex = self.get_hex_at_position(x, y)

        if not clicked_hex:
            return

        if button == arcade.MOUSE_BUTTON_LEFT:
            self.handle_left_click(clicked_hex)
        elif button == arcade.MOUSE_BUTTON_RIGHT:
            self.handle_right_click(clicked_hex)

    def on_mouse_release(self, x, y, button, modifiers):
        """Handle mouse button release"""
        if button == arcade.MOUSE_BUTTON_MIDDLE:
            self.middle_mouse_pressed = False

    def handle_left_click(self, hex_tile):
        """Handle left mouse button click"""
        # If no unit selected, try to select unit
        if not self.selected_unit:
            if hex_tile.unit and hex_tile.unit.owner == self.game_state.current_player.side:
                self.select_unit(hex_tile.unit)
                self.add_message(f"{hex_tile.unit.get_name()} selected")
            elif hex_tile.unit:
                self.add_message(f"Cannot select enemy unit")
        else:
            # Try to move selected unit
            if hex_tile in self.reachable_hexes:
                success, message = self.game_state.move_unit(self.selected_unit, hex_tile)
                self.add_message(message)
                self.deselect_unit()
            else:
                # Clicked on unreachable hex
                if hex_tile.unit:
                    self.add_message(f"Cannot move there - hex occupied")
                else:
                    self.add_message(f"Hex is not reachable")
        self.update_hud()

    def handle_right_click(self, hex_tile):
        """Handle right mouse button click"""
        if self.selected_unit:
            if hex_tile in self.attackable_hexes:
                success, message = self.game_state.attack_unit(self.selected_unit, hex_tile)
                self.add_message(message)
                self.deselect_unit()
            else:
                self.add_message(f"Target is not attackable")
        else:
            self.add_message(f"No unit selected")
        self.update_hud()
    
    def select_unit(self, unit):
        """Select a unit"""
        self.selected_unit = unit
        
        # Calculate reachable hexes
        self.reachable_hexes = self.game_state.hex_map.get_reachable_hexes(
            unit.hex, unit.get_moves_left()
        )
        
        # Remove occupied friendly hexes
        self.reachable_hexes = {
            h: cost for h, cost in self.reachable_hexes.items()
            if not h.unit or h.unit.owner != unit.owner
        }
        
        # Calculate attackable hexes
        self.attackable_hexes = []
        neighbors = self.game_state.hex_map.get_neighbors(unit.hex)
        
        # Artillery has longer range
        if unit.get_class() == UnitClass.ARTILLERY:
            extended = []
            for n in neighbors:
                extended.extend(self.game_state.hex_map.get_neighbors(n))
            neighbors = extended
        
        for hex_tile in neighbors:
            if self.game_state.can_unit_attack(unit, hex_tile):
                self.attackable_hexes.append(hex_tile)
    
    def deselect_unit(self):
        """Deselect current unit"""
        self.selected_unit = None
        self.reachable_hexes = {}
        self.attackable_hexes = []
    
    def get_hex_at_position(self, x, y):
        """Find hex at screen position"""
        # Convert screen position to world position
        # Camera position defines what world position appears at screen (0, 0)
        # Therefore: world_position = screen_position + camera_position
        world_x = x + self.camera_x
        world_y = y + self.camera_y

        # Check each hex
        for row in self.game_state.hex_map.hexes:
            for hex_tile in row:
                hex_x, hex_y = hex_tile.get_pixel_position()
                distance = math.sqrt((world_x - hex_x)**2 + (world_y - hex_y)**2)
                if distance < HEX_SIZE:
                    return hex_tile
        return None
    
    def on_mouse_motion(self, x, y, dx, dy):
        """Track mouse for highlighting and handle middle-click drag"""
        # Handle middle mouse drag for camera panning
        if self.middle_mouse_pressed:
            # Calculate drag delta and update camera position
            # Note: we subtract because dragging right should pan view left
            delta_x = x - self.last_mouse_x
            delta_y = y - self.last_mouse_y
            self.camera_x -= delta_x
            self.camera_y -= delta_y
            self.last_mouse_x = x
            self.last_mouse_y = y
            return

        # Only track mouse in game panel for highlighting
        if x >= self.game_panel_width:
            self.highlighted_hex = None
        else:
            self.highlighted_hex = self.get_hex_at_position(x, y)
    
    def on_key_press(self, key, modifiers):
        """Handle keyboard input"""
        if key == arcade.key.SPACE:
            # End turn
            if not self.game_state.game_over:
                self.deselect_unit()
                current_player = self.game_state.get_current_player()
                self.add_message(f"--- {current_player.get_name()} ends turn ---")
                game_over_msg = self.game_state.end_turn()
                if game_over_msg:
                    self.add_message(game_over_msg)
                else:
                    new_player = self.game_state.get_current_player()
                    self.add_message(f"=== {new_player.get_name()}'s turn begins ===")
                self.update_hud()

        elif key == arcade.key.ESCAPE:
            # Deselect
            if self.selected_unit:
                self.add_message(f"{self.selected_unit.get_name()} deselected")
            self.deselect_unit()
            self.update_hud()

        elif key == arcade.key.R:
            # Restart game
            if self.game_state.game_over:
                self.messages = []
                self.setup()
                self.add_message("=== New game started ===")
                self.add_message(f"=== {self.game_state.get_current_player().get_name()}'s turn ===")

        # Camera pan controls
        elif key in (arcade.key.UP, arcade.key.W):
            self.camera_y -= self.camera_speed
        elif key in (arcade.key.DOWN, arcade.key.S):
            self.camera_y += self.camera_speed
        elif key in (arcade.key.LEFT, arcade.key.A):
            self.camera_x += self.camera_speed
        elif key in (arcade.key.RIGHT, arcade.key.D):
            self.camera_x -= self.camera_speed
    
    def on_update(self, delta_time):
        """Update game logic"""
        if self.game_state.game_over:
            return

        # Process AI turn
        current_player = self.game_state.get_current_player()
        if current_player.player_type == PlayerType.AI:
            if not self.ai_thinking:
                self.ai_thinking = True
                self.ai_delay = 0.5  # Delay so we can see what AI is doing
                self.add_message(f"=== {current_player.get_name()} AI thinking... ===")
                self.update_hud()

            self.ai_delay -= delta_time
            if self.ai_delay <= 0:
                action_type, unit, target = self.ai.get_action()

                if action_type == ActionType.END_TURN:
                    self.add_message(f"--- {current_player.get_name()} AI ends turn ---")
                    game_over_msg = self.game_state.end_turn()
                    self.ai_thinking = False
                    if game_over_msg:
                        self.add_message(game_over_msg)
                    else:
                        new_player = self.game_state.get_current_player()
                        self.add_message(f"=== {new_player.get_name()}'s turn begins ===")
                    self.update_hud()
                elif action_type == ActionType.MOVE:
                    success, message = self.game_state.move_unit(unit, target)
                    self.add_message(f"AI: {message}")
                    self.ai_delay = 0.3  # Small delay between actions
                elif action_type == ActionType.ATTACK:
                    success, message = self.game_state.attack_unit(unit, target)
                    self.add_message(f"AI: {message}")
                    self.ai_delay = 0.3


def main():
    """Main function"""
    game = PanzerGame()
    game.setup()
    arcade.run()


if __name__ == "__main__":
    main()
