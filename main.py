"""
Panzer General 2 Prototype - Main Game
Python Arcade Implementation
"""

import arcade
import math
from constants import *
from game_state import GameState
from ai import SimpleAI

class PanzerGame(arcade.Window):
    """Main game window"""
    
    def __init__(self):
        super().__init__(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_TITLE)
        
        arcade.set_background_color(arcade.color.BLACK)
        
        # Game state
        self.game_state = None
        self.ai = None
        
        # UI state
        self.selected_unit = None
        self.reachable_hexes = {}
        self.attackable_hexes = []
        self.highlighted_hex = None
        
        # Camera
        self.camera_x = 0
        self.camera_y = 0
        
        # Info panel
        self.info_text = ""
        
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
        
    def on_draw(self):
        """Render the game"""
        self.clear()
        
        # Draw map
        self.draw_map()
        
        # Draw units
        self.draw_units()
        
        # Draw UI
        self.draw_ui()
        
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
    
    def draw_ui(self):
        """Draw UI elements"""
        # Draw info panel
        panel_x = SCREEN_WIDTH - 250
        panel_y = SCREEN_HEIGHT - 20
        
        # Turn info
        turn_text = f"Turn: {self.game_state.turn}/{self.game_state.max_turns}"
        arcade.draw_text(turn_text, panel_x, panel_y, arcade.color.WHITE, 14, bold=True)
        
        # Current player
        player = self.game_state.get_current_player()
        player_text = f"Current: {player.get_name()}"
        arcade.draw_text(player_text, panel_x, panel_y - 25, 
                        player.get_color(), 14, bold=True)
        
        # Unit count
        units_text = f"Units: {len(player.get_active_units())}"
        arcade.draw_text(units_text, panel_x, panel_y - 50, 
                        arcade.color.WHITE, 12)
        
        # Selected unit info
        if self.selected_unit:
            info_y = SCREEN_HEIGHT - 150
            arcade.draw_text("Selected Unit:", panel_x, info_y, 
                           arcade.color.WHITE, 12, bold=True)
            
            lines = self.selected_unit.get_info_string().split('\n')
            for i, line in enumerate(lines):
                arcade.draw_text(line, panel_x, info_y - 20 - (i * 15),
                               arcade.color.WHITE, 10)
        
        # Instructions
        inst_y = 150
        instructions = [
            "Left Click: Select/Move unit",
            "Right Click: Attack",
            "SPACE: End turn",
            "ESC: Deselect"
        ]
        
        arcade.draw_text("Controls:", 10, inst_y + 60,
                        arcade.color.WHITE, 12, bold=True)
        for i, inst in enumerate(instructions):
            arcade.draw_text(inst, 10, inst_y + 40 - (i * 15),
                           arcade.color.WHITE, 10)
        
        # AI thinking indicator
        if self.ai_thinking:
            arcade.draw_text("AI Thinking...", SCREEN_WIDTH // 2, 
                           SCREEN_HEIGHT - 50,
                           arcade.color.YELLOW, 16, bold=True,
                           anchor_x="center")
    
    def draw_game_over(self):
        """Draw game over screen"""
        # Semi-transparent overlay - lrbt format (left, right, bottom, top)
        arcade.draw_lrbt_rectangle_filled(
            0, SCREEN_WIDTH,
            0, SCREEN_HEIGHT,
            (0, 0, 0, 200)
        )
        
        # Game over text
        arcade.draw_text("GAME OVER", SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 + 50,
                        arcade.color.WHITE, 48, bold=True,
                        anchor_x="center", anchor_y="center")
        
        # Winner
        winner_text = f"{SIDE_NAMES[self.game_state.winner]} Wins!"
        winner_color = SIDE_COLORS[self.game_state.winner]
        arcade.draw_text(winner_text, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2,
                        winner_color, 36, bold=True,
                        anchor_x="center", anchor_y="center")
        
        # Restart instruction
        arcade.draw_text("Press R to Restart", SCREEN_WIDTH / 2, 
                        SCREEN_HEIGHT / 2 - 50,
                        arcade.color.WHITE, 20,
                        anchor_x="center", anchor_y="center")
    
    def on_mouse_press(self, x, y, button, modifiers):
        """Handle mouse clicks"""
        if self.game_state.game_over:
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
    
    def handle_left_click(self, hex_tile):
        """Handle left mouse button click"""
        # If no unit selected, try to select unit
        if not self.selected_unit:
            if hex_tile.unit and hex_tile.unit.owner == self.game_state.current_player.side:
                self.select_unit(hex_tile.unit)
        else:
            # Try to move selected unit
            if hex_tile in self.reachable_hexes:
                self.game_state.move_unit(self.selected_unit, hex_tile)
                self.deselect_unit()
    
    def handle_right_click(self, hex_tile):
        """Handle right mouse button click"""
        if self.selected_unit and hex_tile in self.attackable_hexes:
            self.game_state.attack_unit(self.selected_unit, hex_tile)
            self.deselect_unit()
    
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
        # Check each hex
        for row in self.game_state.hex_map.hexes:
            for hex_tile in row:
                hex_x, hex_y = hex_tile.get_pixel_position()
                distance = math.sqrt((x - hex_x)**2 + (y - hex_y)**2)
                if distance < HEX_SIZE:
                    return hex_tile
        return None
    
    def on_mouse_motion(self, x, y, dx, dy):
        """Track mouse for highlighting"""
        self.highlighted_hex = self.get_hex_at_position(x, y)
    
    def on_key_press(self, key, modifiers):
        """Handle keyboard input"""
        if key == arcade.key.SPACE:
            # End turn
            if not self.game_state.game_over:
                self.deselect_unit()
                self.game_state.end_turn()
        
        elif key == arcade.key.ESCAPE:
            # Deselect
            self.deselect_unit()
        
        elif key == arcade.key.R:
            # Restart game
            if self.game_state.game_over:
                self.setup()
    
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
            
            self.ai_delay -= delta_time
            if self.ai_delay <= 0:
                action_type, unit, target = self.ai.get_action()
                
                if action_type == ActionType.END_TURN:
                    self.game_state.end_turn()
                    self.ai_thinking = False
                elif action_type == ActionType.MOVE:
                    self.game_state.move_unit(unit, target)
                    self.ai_delay = 0.3  # Small delay between actions
                elif action_type == ActionType.ATTACK:
                    self.game_state.attack_unit(unit, target)
                    self.ai_delay = 0.3


def main():
    """Main function"""
    game = PanzerGame()
    game.setup()
    arcade.run()


if __name__ == "__main__":
    main()
