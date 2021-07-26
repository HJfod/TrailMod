// generated through GDMake https://github.com/HJfod/GDMake

// include GDMake & submodules
#include <GDMake.h>

using namespace gd;
using namespace gdmake;
using namespace gdmake::extra;
using namespace cocos2d;

// TODO: make it so wave streaks and the extra streak save

std::unordered_map<uintptr_t, std::vector<uint8_t>> g_patchedBytes;

void patch(uintptr_t addr, std::vector<uint8_t> bytes) {
    if (!g_patchedBytes[addr].size())
        g_patchedBytes[addr] = patchBytesEx(addr, bytes);
}

void unpatch(uintptr_t addr) {
    if (g_patchedBytes[addr].size())
        patchBytes(addr, g_patchedBytes[addr]);
    
    g_patchedBytes[addr] = {};
}

static constexpr const IconType kIconTypeWaveStreak = static_cast<IconType>(95);
static constexpr const UnlockType kUnlockTypeWaveStreak = static_cast<UnlockType>(95);

enum WaveStreak {
    kWaveStreakDefault = 1,
    kWaveStreakSolid = 2,
    kWaveStreakNone = 3,
    kWaveStreakRainbow = 4,
};

static constexpr const int WaveStreakCount = 3;
WaveStreak g_waveStreak = kWaveStreakDefault;

static constexpr const int HARDSTREAK_MSTREAK_TAG = 5;

class GJGarageLayer_CB : public GJGarageLayer {
    public:
        void onWaveStreak(CCObject* pSender) {
            g_waveStreak = static_cast<WaveStreak>(as<CCNode*>(pSender)->getTag());

            this->m_bUpdateSelector = true;

            if (g_waveStreak == kWaveStreakSolid)
                patch(0x1e8162, { 0x90, 0x90 });
            else
                unpatch(0x1e8162);
        }
};

// i'll figure out fire streaks at some point

/*
GDMAKE_HOOK(0x14e430)
bool __fastcall HardStreak_init(HardStreak* self) {
    if (!GDMAKE_ORIG(self))
        return false;

    // auto s = CCMotionStreak::create(0.4f, 3.0f, 3.0f, { 255, 255, 255 }, "streak_07_001.png");

    // s->setVisible(false);
    // s->setTag(HARDSTREAK_MSTREAK_TAG);

    // self->addChild(s);
    
    return true;
}

//*/

// GDMAKE_HOOK(0x14e530)
// void __fastcall HardStreak_updateStroke(HardStreak* self, float idk) {
//     GDMAKE_ORIG_V(self, idk);

//     if (g_waveStreak == kWaveStreakRainbow);
// }

GDMAKE_HOOK(0x1f9080)
void __fastcall PlayerObject_activateStreak(PlayerObject* self) {
    if (GameManager::sharedState()->getPlayerStreak() == 8 && g_waveStreak == kWaveStreakNone)
        return;

    if (GameManager::sharedState()->getPlayerStreak() == 8 || self->m_isWave)
        patch(0x1f90b9, {
            0x90, 0x90, 0x90, 0x90, 0x90, 0x90, // CALL
        });
    else
        unpatch(0x1f90b9);

    if (g_waveStreak == kWaveStreakNone)
        patch(0x1f90C6, { 0xeb, 0x42 });
    else
        unpatch(0x1f90C6);

    GDMAKE_ORIG(self);
}

GDMAKE_HOOK(0x12ccf0)
bool __fastcall GJItemIcon_init(
    GJItemIcon* self, edx_t edx,
    UnlockType type, int iconID,
    ccColor3B col1, ccColor3B col2,
    bool un0, bool un1, bool un2,
    ccColor3B col3
) {
    if (type == kUnlockTypeSpecial && iconID == 8) {
        if (!GDMAKE_ORIG(self, edx, type, 1, col1, col2, un0, un1, un2, col3))
            return false;
        
        auto xSprite = CCLabelBMFont::create("None", "bigFont.fnt");

        xSprite->setScale(.3f);
        xSprite->setPosition(self->getScaledContentSize() / 2);

        self->addChild(xSprite);
        self->setOpacity(120);

        return true;
    }

    if (type == kUnlockTypeWaveStreak) {
        switch (iconID) {
            default:
                return GDMAKE_ORIG(self, edx, kUnlockTypeSpecial, 1, col1, col2, un0, un1, un2, col3);

            case kWaveStreakRainbow:
                return GDMAKE_ORIG(self, edx, kUnlockTypeSpecial, 2, col1, col2, un0, un1, un2, col3);

            case kWaveStreakNone: {
                if (!GDMAKE_ORIG(self, edx, kUnlockTypeSpecial, 1, col1, col2, un0, un1, un2, col3))
                    return false;

                auto xSprite = CCLabelBMFont::create("None", "bigFont.fnt");

                xSprite->setScale(.3f);
                xSprite->setPosition(self->getScaledContentSize() / 2);

                self->addChild(xSprite);
                self->setOpacity(120);
                
                return true;
            }
        }
    }
    
    return GDMAKE_ORIG(self, edx, type, iconID, col1, col2, un0, un1, un2, col3);
}

GDMAKE_HOOK(0xc4fc0)
bool __fastcall GameManager_isIconUnlocked(GameManager* self, edx_t edx, int iconID, IconType type) {
    if (type == kIconTypeWaveStreak)
        return true;

    if (type == kIconTypeSpecial && iconID == 8)
        return true;
    
    return GDMAKE_ORIG(self, edx, iconID, type);
}

GDMAKE_HOOK(0x12af70)
bool __fastcall GaragePage_init(GaragePage* self, edx_t edx, IconType type, GJGarageLayer* pLayer, SEL_MenuHandler pHandler) {
    if (type == kIconTypeWaveStreak) {
        // patch unlock type
        auto unlockTypeBytes = patchBytesEx(0x12b200, { 0xbf, kUnlockTypeWaveStreak, 0x00, 0x00, 0x00 });
        // patch number of items
        auto iconCountBytes = patchBytesEx(0x12b205, {
            0xc7, 0x44, 0x24, 0x1c, WaveStreakCount, 0x00, 0x00, 0x00
        });
        // patch it to test against some other number than 0x63
        // so it results in false
        auto showGlowBtnBytes = patchBytesEx(0x12b7e0, { 0x83, 0x7d, 0x08, 0x00 });
        // patch switch jump to go to kIconTypeSpecial
        auto switchJmpBytes = patchBytesEx(0x12b089, { 0xe9, 0x72, 0x01, 0x00, 0x00, 0x90, 0x90 });
        // 
        auto defaultItem = patchBytesEx(0x12b21f, {
            0xbb, static_cast<uint8_t>(g_waveStreak), 0x00, 0x00, 0x00,
            0x90, 0x90, 0x90, 0x90, 0x90, 0x90,
            0x90, 0x90, 0x90, 0x90, 0x90, 0x90,
        });

        auto ret = GDMAKE_ORIG(self, edx, kIconTypeWaveStreak, pLayer, pHandler);

        // reset patches
        patchBytes(0x12b21f, defaultItem);
        patchBytes(0x12b089, switchJmpBytes);
        patchBytes(0x12b7e0, showGlowBtnBytes);
        patchBytes(0x12b200, unlockTypeBytes);
        patchBytes(0x12b205, iconCountBytes);

        return ret;
    }

    if (!GDMAKE_ORIG(self, edx, type, pLayer, pHandler))
        return false;

    return true;
}

GDMAKE_HOOK(0x127c50)
void __fastcall GJGarageLayer_selectPage(GJGarageLayer* self, edx_t edx, IconType type) {
    auto pMenu = as<CCMenu*>(self->m_pTabToggleCube->getParent());
    auto pToggle = as<CCMenuItemToggler*>(pMenu->getChildByTag(kIconTypeWaveStreak));

    if (type == kIconTypeWaveStreak) {
        auto bytes = patchBytesEx(0x127c7a, { 0xbf, 0x9, 0x00, 0x00, 0x00 });

        GDMAKE_ORIG(self, edx, kIconTypeShip);

        self->m_pTabToggleShip->toggle(false);
        self->m_pTabToggleShip->setEnabled(true);

        pToggle->toggle(true);
        pToggle->setEnabled(false);

       patchBytes(0x127c7a, bytes);
    } else {
        GDMAKE_ORIG(self, edx, type);

        pToggle->toggle(false);
        pToggle->setEnabled(true);
    }
}

GDMAKE_HOOK(0x1255d0)
bool __fastcall GJGarageLayer_init(GJGarageLayer* self) {
    if (!GDMAKE_ORIG(self))
        return false;

    auto pMenu = as<CCMenu*>(self->m_pTabToggleCube->getParent());

    auto pPage = GaragePage::create(kIconTypeWaveStreak, self, (SEL_MenuHandler)&GJGarageLayer_CB::onWaveStreak);

    self->addChild(pPage, 100);
    self->m_pPagesArray->addObject(pPage);

    pPage->setVisible(false);

    auto pButton = CCMenuItemToggler::createWithSize(
        "gj_streakBtn_off_001.png",
        "gj_streakBtn_on_001.png",
        self, (SEL_MenuHandler)&GJGarageLayer::onSelectTab,
        .9f
    );

    pButton->setSizeMult(1.2f);
    pButton->setTag(kIconTypeWaveStreak);

    pMenu->addChild(pButton);
    pMenu->alignItemsHorizontallyWithPadding(-4.0f);
    pMenu->setPositionX(self->getContentSize().width / 2);

    return true;
}

GDMAKE_MAIN {
    // change icontype
    patch(0x12b205, {
        0xc7, 0x44, 0x24, 0x1c, 0x08, 0x00, 0x00, 0x00
    });

    // delete the dumb "tap for more info" text
    patch(0x12706d, {
        0x90, 0x90, 0x90, 0x90, 0x90,                   // PUSH
        0x90, 0x90,                                     // CALL
        0x90, 0x90, 0x90, 0x90,                         // MOV
        0x90, 0x90, 0x90,                               // ADD
        0x90, 0x90,                                     // MOV
        0x90, 0x90,                                     // MOV
        0x90,                                           // PUSH
        0x90, 0x90, 0x90, 0x90, 0x90, 0x90,             // CALL
        0x90, 0x90,                                     // MOV
        0x90, 0x90, 0x90, 0x90,                         // LEA
        0x90, 0x90, 0x90,                               // SUB
        0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, // MOV
        0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,       // MOV
        0x90, 0x90, 0x90, 0x90, 0x90, 0x90,             // CALL
        0x90,                                           // PUSH
        0x90, 0x90,                                     // MOV
        0x90, 0x90, 0x90, 0x90, 0x90, 0x90,             // CALL
        0x90, 0x90, 0x90, 0x90, 0x90, 0x90,             // MOVSS
        0x90, 0x90, 0x90, 0x90,                         // LEA
        0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, // MULSS
        0x90, 0x90, 0x90,                               // SUB
        0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, // ADDSS
        0x90, 0x90, 0x90, 0x90, 0x90, 0x90,             // MOVSS
        0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,       // MOV
        0x90, 0x90, 0x90, 0x90, 0x90, 0x90,             // CALL
        0x90, 0x90,                                     // MOV
        0x90, 0x90, 0x90, 0x90,                         // MOV
        0x90, 0x90,                                     // MOV
        0x90, 0x90, 0x90,                               // CALL
        0x90,                                           // PUSH
        0x90, 0x90, 0x90, 0x90,                         // LEA
        0x90,                                           // PUSH
        0x90, 0x90,                                     // MOV
        0x90, 0x90, 0x90, 0x90, 0x90, 0x90,             // CALL
        0x90, 0x90,                                     // MOV
        0x90, 0x90, 0x90, 0x90,                         // LEA
        0x90,                                           // PUSH
        0x90, 0x90,                                     // MOV
        0x90, 0x90, 0x90,                               // CALL
    });

    return true;
}

GDMAKE_UNLOAD {
    // unpatch bytes
    unpatch(0x12b205);
    unpatch(0x12706d);
    unpatch(0x1f90b9);
    unpatch(0x1f90C6);
}
