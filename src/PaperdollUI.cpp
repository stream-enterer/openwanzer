#include "PaperdollUI.hpp"
#include "ArmorLocation.hpp"
#include "Constants.hpp"
#include "GameLogic.hpp"
#include "raylib.h"
#include "raymath.h"
#include "raygui.h"
#include <string>
#include <cmath>

namespace paperdollui {

// ============================================================================
// COLORS AND CONSTANTS
// ============================================================================

// Armor health state colors (based on percentage)
Color getArmorColor(float percentage) {
    if (percentage >= 0.80f) return WHITE;
    else if (percentage >= 0.60f) return LIGHTGRAY;
    else if (percentage >= 0.40f) return GRAY;
    else if (percentage >= 0.20f) return DARKGRAY;
    else return Color{50, 50, 50, 255};
}

// Structure exposed state
const Color STRUCTURE_COLOR = ORANGE;
const int STRIPE_WIDTH = 3;

// Destroyed location
const Color DESTROYED_COLOR = BLACK;
const Color DESTROYED_OUTLINE = Color{40, 40, 40, 255};

// Weapon type colors
const Color MISSILE_COLOR = MAGENTA;
const Color ENERGY_COLOR = GREEN;
const Color BALLISTIC_COLOR = SKYBLUE;  // Cyan
const Color ARTILLERY_COLOR = RED;
const Color MELEE_COLOR = WHITE;
const Color DISABLED_WEAPON_COLOR = DARKGRAY;

// Attack arc indicator
const Color ARC_INDICATOR_COLOR = RED;
const int ARC_LINE_THICKNESS = 2;
const int ARC_LINE_PADDING = 2;

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

Color getWeaponColor(WeaponType type) {
    switch (type) {
        case WeaponType::MISSILE: return MISSILE_COLOR;
        case WeaponType::ENERGY: return ENERGY_COLOR;
        case WeaponType::BALLISTIC: return BALLISTIC_COLOR;
        case WeaponType::ARTILLERY: return ARTILLERY_COLOR;
        case WeaponType::MELEE: return MELEE_COLOR;
        default: return WHITE;
    }
}

std::string getWeightClassName(Unit::WeightClass wClass) {
    switch (wClass) {
        case Unit::WeightClass::LIGHT: return "LIGHT";
        case Unit::WeightClass::MEDIUM: return "MEDIUM";
        case Unit::WeightClass::HEAVY: return "HEAVY";
        case Unit::WeightClass::ASSAULT: return "ASSAULT";
        default: return "UNKNOWN";
    }
}

// Draw diagonal stripes for structure exposed
void renderDiagonalStripes(Rectangle rect, Color stripeColor, int width) {
    for (int offset = -(int)rect.height; offset < rect.width; offset += width * 2) {
        Vector2 start = {rect.x + offset, rect.y};
        Vector2 end = {rect.x + offset + rect.height, rect.y + rect.height};
        DrawLineEx(start, end, (float)width, stripeColor);
    }
}

// ============================================================================
// BODY SECTION RENDERING
// ============================================================================

void renderBodySection(Rectangle rect, const Unit* unit, ArmorLocation location) {
    if (unit->locations.find(location) == unit->locations.end()) {
        return;
    }

    const LocationStatus& loc = unit->locations.at(location);

    // Determine state and color
    bool isDestroyed = (loc.currentArmor == 0 && loc.currentStructure == 0);
    bool isStructureExposed = (loc.currentArmor == 0 && loc.currentStructure > 0);

    if (isDestroyed) {
        // Black with grey outline
        DrawRectangleRec(rect, DESTROYED_COLOR);
        DrawRectangleLinesEx(rect, 1, DESTROYED_OUTLINE);
    }
    else if (isStructureExposed) {
        // Orange with diagonal stripes
        DrawRectangleRec(rect, STRUCTURE_COLOR);
        renderDiagonalStripes(rect, BLACK, STRIPE_WIDTH);
        DrawRectangleLinesEx(rect, 1, DESTROYED_OUTLINE);
    }
    else {
        // Armor present - color based on percentage
        float armorPercent = (float)loc.currentArmor / loc.maxArmor;
        Color armorColor = getArmorColor(armorPercent);
        DrawRectangleRec(rect, armorColor);
        DrawRectangleLinesEx(rect, 1, Color{40, 40, 40, 255});
    }
}

// ============================================================================
// PAPERDOLL RENDERING
// ============================================================================

void renderFrontPaperdoll(const PaperdollPanel& panel, const Unit* unit) {
    // Draw each body section with appropriate color
    renderBodySection(panel.frontHead, unit, ArmorLocation::HEAD);
    renderBodySection(panel.frontLA, unit, ArmorLocation::LEFT_ARM);
    renderBodySection(panel.frontRA, unit, ArmorLocation::RIGHT_ARM);
    renderBodySection(panel.frontLT, unit, ArmorLocation::LEFT_TORSO);
    renderBodySection(panel.frontCT, unit, ArmorLocation::CENTER_TORSO);
    renderBodySection(panel.frontRT, unit, ArmorLocation::RIGHT_TORSO);
    renderBodySection(panel.frontLL, unit, ArmorLocation::LEFT_LEG);
    renderBodySection(panel.frontRL, unit, ArmorLocation::RIGHT_LEG);
}

void renderRearPaperdoll(const PaperdollPanel& panel, const Unit* unit) {
    // Draw rear torso sections
    renderBodySection(panel.rearLT, unit, ArmorLocation::LEFT_TORSO_REAR);
    renderBodySection(panel.rearCT, unit, ArmorLocation::CENTER_TORSO_REAR);
    renderBodySection(panel.rearRT, unit, ArmorLocation::RIGHT_TORSO_REAR);

    // Draw blackened arms (non-targetable from rear)
    DrawRectangleRec(panel.rearLA, Color{20, 20, 20, 255});
    DrawRectangleLinesEx(panel.rearLA, 1, Color{40, 40, 40, 255});

    DrawRectangleRec(panel.rearRA, Color{20, 20, 20, 255});
    DrawRectangleLinesEx(panel.rearRA, 1, Color{40, 40, 40, 255});
}

void renderPaperdollLabels(const PaperdollPanel& panel) {
    // Draw "FRONT" and "REAR" labels below paperdolls
    float frontLabelX = panel.frontCT.x + panel.frontCT.width / 2 - 20;
    float rearLabelX = panel.rearCT.x + panel.rearCT.width / 2 - 20;
    float labelY = panel.frontLL.y + panel.frontLL.height + 5;

    DrawText("FRONT", (int)frontLabelX, (int)labelY, 10, GRAY);
    DrawText("REAR", (int)rearLabelX, (int)labelY, 10, GRAY);
}

// ============================================================================
// ATTACK ARC INDICATORS
// ============================================================================

Rectangle getBoundingRectForFrontPaperdoll(const PaperdollPanel& panel) {
    // Calculate bounding box for front paperdoll
    float minX = panel.frontLA.x;
    float maxX = panel.frontRA.x + panel.frontRA.width;
    float minY = panel.frontHead.y;
    float maxY = panel.frontLL.y + panel.frontLL.height;
    return Rectangle{minX, minY, maxX - minX, maxY - minY};
}

Rectangle getBoundingRectForRearPaperdoll(const PaperdollPanel& panel) {
    // Calculate bounding box for rear paperdoll
    float minX = panel.rearLA.x;
    float maxX = panel.rearRA.x + panel.rearRA.width;
    float minY = panel.rearLT.y;
    float maxY = panel.rearRT.y + panel.rearRT.height;
    return Rectangle{minX, minY, maxX - minX, maxY - minY};
}

void drawVerticalArcLine(Rectangle bounds, bool isLeftSide) {
    float x = isLeftSide ?
        (bounds.x - ARC_LINE_PADDING - ARC_LINE_THICKNESS) :
        (bounds.x + bounds.width + ARC_LINE_PADDING);

    Vector2 start = {x, bounds.y};
    Vector2 end = {x, bounds.y + bounds.height};

    // Draw double line
    DrawLineEx(start, end, (float)ARC_LINE_THICKNESS, ARC_INDICATOR_COLOR);
    DrawLineEx(Vector2{x + ARC_LINE_THICKNESS + 1, start.y},
               Vector2{x + ARC_LINE_THICKNESS + 1, end.y},
               (float)ARC_LINE_THICKNESS, ARC_INDICATOR_COLOR);
}

void renderAttackArcIndicators(const PaperdollPanel& panel, combatarcs::AttackArc arc) {
    Rectangle frontBounds = getBoundingRectForFrontPaperdoll(panel);
    Rectangle rearBounds = getBoundingRectForRearPaperdoll(panel);

    switch (arc) {
        case combatarcs::AttackArc::FRONT:
            // Lines on left and right of front paperdoll
            drawVerticalArcLine(frontBounds, true);   // left
            drawVerticalArcLine(frontBounds, false);  // right
            break;

        case combatarcs::AttackArc::LEFT_SIDE:
            // Line on left side of front paperdoll only
            drawVerticalArcLine(frontBounds, true);
            break;

        case combatarcs::AttackArc::RIGHT_SIDE:
            // Line on right side of front paperdoll only
            drawVerticalArcLine(frontBounds, false);
            break;

        case combatarcs::AttackArc::REAR:
            // Lines on both sides of rear paperdoll
            drawVerticalArcLine(rearBounds, true);
            drawVerticalArcLine(rearBounds, false);
            break;
    }
}

// ============================================================================
// WEAPON LOADOUT
// ============================================================================

void renderWeaponLoadout(const PaperdollPanel& panel, const Unit* unit) {
    float x = panel.bounds.x + panel.bounds.width - 150;  // Right side
    float y = panel.bounds.y + 30;

    DrawText("LOADOUT", (int)x, (int)y, 14, GRAY);
    y += 20;

    for (const Weapon& weapon : unit->weapons) {
        Color weaponColor = getWeaponColor(weapon.type);
        if (weapon.isDestroyed) {
            weaponColor = DISABLED_WEAPON_COLOR;
        }

        DrawText(weapon.name.c_str(), (int)x, (int)y, 12, weaponColor);

        // Draw damage number in grey next to weapon
        if (weapon.type == WeaponType::MELEE || weapon.type == WeaponType::ARTILLERY) {
            DrawText(TextFormat("%d", weapon.damage), (int)(x + 80), (int)y, 12, GRAY);
        }

        y += 16;
    }
}

// ============================================================================
// STATUS BARS
// ============================================================================

void renderStatusBar(float x, float y, float width, float height,
                     int current, int max, Color barColor, const char* label) {
    // Background
    DrawRectangle((int)x, (int)y, (int)width, (int)height, DARKGRAY);

    // Filled portion
    float fillWidth = (max > 0) ? (width * current) / max : 0;
    DrawRectangle((int)x, (int)y, (int)fillWidth, (int)height, barColor);

    // Border
    DrawRectangleLines((int)x, (int)y, (int)width, (int)height, WHITE);

    // Text
    DrawText(TextFormat("%d/%d", current, max), (int)(x + 5), (int)(y + 3), 10, WHITE);
    DrawText(label, (int)x, (int)(y + height + 2), 8, GRAY);
}

// ============================================================================
// PANEL HEADER
// ============================================================================

void renderPanelHeader(const PaperdollPanel& panel, const Unit* unit, [[maybe_unused]] bool isTargetPanel) {
    float x = panel.bounds.x + 10;
    float y = panel.bounds.y + 10;

    // Line 1: Mech name and variant
    std::string mechName = getWeightClassName(unit->weightClass);
    std::string variant = "MK-I";  // Placeholder
    DrawText(TextFormat("%s - %s", mechName.c_str(), variant.c_str()),
             (int)x, (int)y, 20, ORANGE);

    // Line 1 continued: S: and A: values
    int totalStructure = 0, currentStructure = 0;
    int totalArmor = 0, currentArmor = 0;

    for (const auto& loc : unit->locations) {
        totalArmor += loc.second.maxArmor;
        currentArmor += loc.second.currentArmor;
        totalStructure += loc.second.maxStructure;
        currentStructure += loc.second.currentStructure;
    }

    DrawText(TextFormat("S: %d/%d", currentStructure, totalStructure),
             (int)(x + 300), (int)y, 16, WHITE);
    DrawText(TextFormat("A: %d/%d", currentArmor, totalArmor),
             (int)(x + 450), (int)y, 16, WHITE);

    // Line 2: Mech weight class and faction/pilot info
    y += 25;
    DrawText(TextFormat("'MECH: %s", mechName.c_str()), (int)x, (int)y, 14, LIGHTGRAY);

    // Placeholder faction logo (just a small box)
    DrawRectangle((int)(x + 150), (int)y, 20, 20, DARKGRAY);
    DrawText("PILOT", (int)(x + 180), (int)y, 14, LIGHTGRAY);

    // Line 3: Initiative, Heat, Stability bars
    y += 25;

    // Initiative (placeholder)
    DrawText("-1", (int)x, (int)y, 14, WHITE);
    DrawText("INITIATIVE", (int)x, (int)(y + 15), 10, LIGHTGRAY);

    // Heat bar (placeholder values)
    renderStatusBar(x + 80, y, 100, 20, 50, 500, RED, "HEAT");

    // Stability bar (placeholder values)
    renderStatusBar(x + 200, y, 100, 20, 75, 100, YELLOW, "STAB");
}

// ============================================================================
// TOOLTIPS
// ============================================================================

void renderLocationTooltip(const PaperdollPanel& panel, const Unit* unit) {
    if (panel.hoveredLocation == ArmorLocation::NONE) return;

    if (unit->locations.find(panel.hoveredLocation) == unit->locations.end()) {
        return;
    }

    const LocationStatus& loc = unit->locations.at(panel.hoveredLocation);

    // Build tooltip text
    std::string locationName = locationToString(panel.hoveredLocation);
    std::string armorText = TextFormat("%d/%d", loc.currentArmor, loc.maxArmor);
    std::string structureText = TextFormat("%d/%d", loc.currentStructure, loc.maxStructure);

    // Calculate tooltip size
    int tooltipWidth = 120;
    int tooltipHeight = 60;

    // Position near mouse (with bounds checking)
    Vector2 pos = panel.tooltipPos;
    pos.x = Clamp(pos.x, 0, SCREEN_WIDTH - tooltipWidth);
    pos.y = Clamp(pos.y, 0, SCREEN_HEIGHT - tooltipHeight);

    Rectangle tooltipRect = {pos.x, pos.y, (float)tooltipWidth, (float)tooltipHeight};

    // Draw tooltip background
    DrawRectangleRec(tooltipRect, Color{10, 10, 10, 240});
    DrawRectangleLinesEx(tooltipRect, 1, LIGHTGRAY);

    // Draw text
    DrawText(locationName.c_str(), (int)(pos.x + 5), (int)(pos.y + 5), 12, WHITE);

    bool isStructureExposed = (loc.currentArmor == 0 && loc.currentStructure > 0);
    Color structColor = isStructureExposed ? ORANGE : WHITE;

    DrawText(TextFormat("A: %s", armorText.c_str()),
             (int)(pos.x + 5), (int)(pos.y + 22), 10, WHITE);
    DrawText(TextFormat("S: %s", structureText.c_str()),
             (int)(pos.x + 5), (int)(pos.y + 37), 10, structColor);
}

// ============================================================================
// MAIN PANEL RENDERING
// ============================================================================

void renderTargetPanel(const GameState& game) {
    if (!game.targetPanel.isVisible || !game.targetPanel.targetUnit) return;

    const TargetPanel& panel = game.targetPanel;
    Unit* unit = panel.targetUnit;

    // 1. Draw panel background
    Color backgroundColor = GetColor(GuiGetStyle(DROPDOWNBOX, BASE_COLOR_NORMAL));
    Color borderColor = GetColor(GuiGetStyle(DROPDOWNBOX, BORDER_COLOR_NORMAL));

    DrawRectangleRec(panel.bounds, backgroundColor);
    DrawRectangleLinesEx(panel.bounds, 2, borderColor);

    // 2. Draw header section
    renderPanelHeader(panel, unit, true);

    // 3. Draw front paperdoll
    renderFrontPaperdoll(panel, unit);

    // 4. Draw rear paperdoll
    renderRearPaperdoll(panel, unit);

    // 5. Draw attack arc indicators (red lines)
    renderAttackArcIndicators(panel, panel.currentArc);

    // 6. Draw "FRONT" and "REAR" labels
    renderPaperdollLabels(panel);

    // 7. Draw weapon loadout list
    renderWeaponLoadout(panel, unit);

    // 8. Draw tooltip if hovering over body part
    if (panel.showTooltip) {
        renderLocationTooltip(panel, unit);
    }
}

void renderPlayerPanel(const GameState& game) {
    if (!game.playerPanel.isVisible || !game.playerPanel.playerUnit) return;

    const PlayerPanel& panel = game.playerPanel;
    Unit* unit = panel.playerUnit;

    // 1. Draw panel background
    Color backgroundColor = GetColor(GuiGetStyle(DROPDOWNBOX, BASE_COLOR_NORMAL));
    Color borderColor = GetColor(GuiGetStyle(DROPDOWNBOX, BORDER_COLOR_NORMAL));

    DrawRectangleRec(panel.bounds, backgroundColor);
    DrawRectangleLinesEx(panel.bounds, 2, borderColor);

    // 2. Draw header section (simplified for player panel)
    renderPanelHeader(panel, unit, false);

    // 3. Draw front paperdoll
    renderFrontPaperdoll(panel, unit);

    // 4. Draw rear paperdoll
    renderRearPaperdoll(panel, unit);

    // 5. Draw "FRONT" and "REAR" labels
    renderPaperdollLabels(panel);

    // 6. Draw tooltip if hovering over body part
    if (panel.showTooltip) {
        renderLocationTooltip(panel, unit);
    }
}

// ============================================================================
// INPUT HANDLING
// ============================================================================

void updatePanelTooltip(PaperdollPanel& panel, Vector2 mousePos) {
    panel.hoveredLocation = ArmorLocation::NONE;
    panel.showTooltip = false;

    // Check each body section for hover
    if (CheckCollisionPointRec(mousePos, panel.frontHead)) {
        panel.hoveredLocation = ArmorLocation::HEAD;
    }
    else if (CheckCollisionPointRec(mousePos, panel.frontCT)) {
        panel.hoveredLocation = ArmorLocation::CENTER_TORSO;
    }
    else if (CheckCollisionPointRec(mousePos, panel.frontLT)) {
        panel.hoveredLocation = ArmorLocation::LEFT_TORSO;
    }
    else if (CheckCollisionPointRec(mousePos, panel.frontRT)) {
        panel.hoveredLocation = ArmorLocation::RIGHT_TORSO;
    }
    else if (CheckCollisionPointRec(mousePos, panel.frontLA)) {
        panel.hoveredLocation = ArmorLocation::LEFT_ARM;
    }
    else if (CheckCollisionPointRec(mousePos, panel.frontRA)) {
        panel.hoveredLocation = ArmorLocation::RIGHT_ARM;
    }
    else if (CheckCollisionPointRec(mousePos, panel.frontLL)) {
        panel.hoveredLocation = ArmorLocation::LEFT_LEG;
    }
    else if (CheckCollisionPointRec(mousePos, panel.frontRL)) {
        panel.hoveredLocation = ArmorLocation::RIGHT_LEG;
    }
    // Rear paperdoll sections
    else if (CheckCollisionPointRec(mousePos, panel.rearCT)) {
        panel.hoveredLocation = ArmorLocation::CENTER_TORSO_REAR;
    }
    else if (CheckCollisionPointRec(mousePos, panel.rearLT)) {
        panel.hoveredLocation = ArmorLocation::LEFT_TORSO_REAR;
    }
    else if (CheckCollisionPointRec(mousePos, panel.rearRT)) {
        panel.hoveredLocation = ArmorLocation::RIGHT_TORSO_REAR;
    }

    if (panel.hoveredLocation != ArmorLocation::NONE) {
        panel.showTooltip = true;
        panel.tooltipPos = mousePos;
    }
}

void handlePaperdollPanelDrag(GameState& game) {
    Vector2 mousePos = GetMousePosition();

    // Start dragging on left click
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        // Check target panel
        if (game.targetPanel.isVisible &&
            CheckCollisionPointRec(mousePos, game.targetPanel.bounds)) {
            game.targetPanel.isDragging = true;
            game.targetPanel.dragOffset.x = mousePos.x - game.targetPanel.bounds.x;
            game.targetPanel.dragOffset.y = mousePos.y - game.targetPanel.bounds.y;
        }

        // Check player panel
        if (game.playerPanel.isVisible &&
            CheckCollisionPointRec(mousePos, game.playerPanel.bounds)) {
            game.playerPanel.isDragging = true;
            game.playerPanel.dragOffset.x = mousePos.x - game.playerPanel.bounds.x;
            game.playerPanel.dragOffset.y = mousePos.y - game.playerPanel.bounds.y;
        }
    }

    // Stop dragging on release
    if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
        game.targetPanel.isDragging = false;
        game.playerPanel.isDragging = false;
    }

    // Update position while dragging
    if (game.targetPanel.isDragging) {
        game.targetPanel.bounds.x = mousePos.x - game.targetPanel.dragOffset.x;
        game.targetPanel.bounds.y = mousePos.y - game.targetPanel.dragOffset.y;
    }

    if (game.playerPanel.isDragging) {
        game.playerPanel.bounds.x = mousePos.x - game.playerPanel.dragOffset.x;
        game.playerPanel.bounds.y = mousePos.y - game.playerPanel.dragOffset.y;
    }
}

void handlePaperdollTooltips(GameState& game) {
    Vector2 mousePos = GetMousePosition();

    // Update target panel tooltips
    if (game.targetPanel.isVisible) {
        updatePanelTooltip(game.targetPanel, mousePos);
    }

    // Update player panel tooltips
    if (game.playerPanel.isVisible) {
        updatePanelTooltip(game.playerPanel, mousePos);
    }
}

} // namespace paperdollui
