#include "../../../pchdef.h"
#include "../../playerbot.h"
#include "../Strategy.h"
#include "MageActions.h"
#include "MageAiObjectContext.h"
#include "FrostMageStrategy.h"
#include "ArcaneMageStrategy.h"
#include "GenericMageNonCombatStrategy.h"
#include "FireMageStrategy.h"
#include "../generic/PullStrategy.h"
#include "MageTriggers.h"
#include "../NamedObjectContext.h"

using namespace ai;


namespace ai
{
    namespace mage
    {
        using namespace ai;

        class StrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            StrategyFactoryInternal()
            {
                creators["nc"] = &mage::StrategyFactoryInternal::nc;
                creators["pull"] = &mage::StrategyFactoryInternal::pull;
                creators["fire aoe"] = &mage::StrategyFactoryInternal::fire_aoe;
                creators["frost aoe"] = &mage::StrategyFactoryInternal::frost_aoe;
            }
            ~StrategyFactoryInternal()
            {
                creators.erase("nc");
                creators.erase("pull");
                creators.erase("fire aoe");
                creators.erase("frost aoe");
            }

        private:
            static Strategy* nc(PlayerbotAI* ai) { return new GenericMageNonCombatStrategy(ai); }
            static Strategy* pull(PlayerbotAI* ai) { return new PullStrategy(ai, "shoot"); }
            static Strategy* fire_aoe(PlayerbotAI* ai) { return new FireMageAoeStrategy(ai); }
            static Strategy* frost_aoe(PlayerbotAI* ai) { return new FrostMageAoeStrategy(ai); }
        };

        class MageStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            MageStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["frost"] = &mage::MageStrategyFactoryInternal::frost;
                creators["fire"] = &mage::MageStrategyFactoryInternal::fire;
                creators["arcane"] = &mage::MageStrategyFactoryInternal::arcane;
            }
            ~MageStrategyFactoryInternal()
            {
                creators.erase("frost");
                creators.erase("fire");
                creators.erase("arcane");
            }
        private:
            static Strategy* frost(PlayerbotAI* ai) { return new FrostMageStrategy(ai); }
            static Strategy* fire(PlayerbotAI* ai) { return new FireMageStrategy(ai); }
            static Strategy* arcane(PlayerbotAI* ai) { return new ArcaneMageStrategy(ai); }
        };

        class MageBuffStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            MageBuffStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["bmana"] = &mage::MageBuffStrategyFactoryInternal::bmana;
                creators["bdps"] = &mage::MageBuffStrategyFactoryInternal::bdps;
            }
            ~MageBuffStrategyFactoryInternal()
            {
                creators.erase("bmana");
                creators.erase("bdps");
            }
        private:
            static Strategy* bmana(PlayerbotAI* ai) { return new MageBuffManaStrategy(ai); }
            static Strategy* bdps(PlayerbotAI* ai) { return new MageBuffDpsStrategy(ai); }
        };
    };
};


namespace ai
{
    namespace mage
    {
        using namespace ai;

        class TriggerFactoryInternal : public NamedObjectContext<Trigger>
        {
        public:
            TriggerFactoryInternal()
            {
               // creators["fireball"] = &TriggerFactoryInternal::fireball;
                //creators["pyroblast"] = &TriggerFactoryInternal::pyroblast;
                creators["combustion"] = &TriggerFactoryInternal::combustion;
                creators["icy veins"] = &TriggerFactoryInternal::icy_veins;
                creators["arcane intellect"] = &TriggerFactoryInternal::arcane_intellect;
                creators["arcane intellect on party"] = &TriggerFactoryInternal::arcane_intellect_on_party;
                creators["molten armor"] = &TriggerFactoryInternal::molten_armor;
                creators["remove curse"] = &TriggerFactoryInternal::remove_curse;
                creators["remove curse on party"] = &TriggerFactoryInternal::remove_curse_on_party;
                creators["counterspell"] = &TriggerFactoryInternal::counterspell;
                creators["polymorph"] = &TriggerFactoryInternal::polymorph;
                creators["spellsteal"] = &TriggerFactoryInternal::spellsteal;
                creators["hot streak"] = &TriggerFactoryInternal::hot_streak;
                creators["blazing speed"] = &TriggerFactoryInternal::blazing_speed;
                creators["fingers of frost"] = &TriggerFactoryInternal::fingers_frost;
                creators["brain freeze"] = &TriggerFactoryInternal::brain_freeze;
                creators["fiery payback"] = &TriggerFactoryInternal::fiery_payback;
                creators["living bomb"] = &TriggerFactoryInternal::living_bomb;
                creators["missile barrage"] = &TriggerFactoryInternal::missile_barrage;
                creators["arcane blast"] = &TriggerFactoryInternal::arcane_blast;
                creators["counterspell on enemy healer"] = &TriggerFactoryInternal::counterspell_enemy_healer;
                creators["slow on attacker"] = &TriggerFactoryInternal::slow;
            }
            ~TriggerFactoryInternal()
            {
                creators.erase("combustion");
                creators.erase("icy veins");
                creators.erase("arcane intellect");
                creators.erase("arcane intellect on party");
                creators.erase("molten armor");
                creators.erase("remove curse");
                creators.erase("remove curse on party");
                creators.erase("counterspell");
                creators.erase("polymorph");
                creators.erase("spellsteal");
                creators.erase("hot streak");
                creators.erase("blazing speed");
                creators.erase("fingers of frost");
                creators.erase("brain freeze");
                creators.erase("fiery payback");
                creators.erase("living bomb");
                creators.erase("missile barrage");
                creators.erase("arcane blast");
                creators.erase("counterspell on enemy healer");
                creators.erase("slow on attacker");
            }
        private:
            static Trigger* hot_streak(PlayerbotAI* ai) { return new HotStreakTrigger(ai); }
            static Trigger* blazing_speed(PlayerbotAI* ai) { return new BlazingSpeedTrigger(ai); }
            static Trigger* fingers_frost(PlayerbotAI* ai) { return new FingersFrostTrigger(ai); }
            static Trigger* brain_freeze(PlayerbotAI* ai) { return new BrainfreezeTrigger(ai); }
            static Trigger* fiery_payback(PlayerbotAI* ai) { return new FieryPaybackTrigger(ai); }
            static Trigger* improved_scorch(PlayerbotAI* ai) { return new ImprovedScorchTrigger(ai); }
            static Trigger* combustion(PlayerbotAI* ai) { return new CombustionTrigger(ai); }
            static Trigger* icy_veins(PlayerbotAI* ai) { return new IcyVeinsTrigger(ai); }
            static Trigger* arcane_intellect(PlayerbotAI* ai) { return new ArcaneIntellectTrigger(ai); }
            static Trigger* arcane_intellect_on_party(PlayerbotAI* ai) { return new ArcaneIntellectOnPartyTrigger(ai); }
            static Trigger* molten_armor(PlayerbotAI* ai) { return new MoltenArmorTrigger(ai); }
            static Trigger* remove_curse(PlayerbotAI* ai) { return new RemoveCurseTrigger(ai); }
            static Trigger* remove_curse_on_party(PlayerbotAI* ai) { return new PartyMemberRemoveCurseTrigger(ai); }
            static Trigger* counterspell(PlayerbotAI* ai) { return new CounterspellInterruptSpellTrigger(ai); }
            static Trigger* polymorph(PlayerbotAI* ai) { return new PolymorphTrigger(ai); }
            static Trigger* spellsteal(PlayerbotAI* ai) { return new SpellstealTrigger(ai); }
            static Trigger* living_bomb(PlayerbotAI* ai) { return new LivingBombTrigger(ai); }
            static Trigger* missile_barrage(PlayerbotAI* ai) { return new MissileBarrageTrigger(ai); }
            static Trigger* arcane_blast(PlayerbotAI* ai) { return new ArcaneBlastTrigger(ai); }
            static Trigger* counterspell_enemy_healer(PlayerbotAI* ai) { return new CounterspellEnemyHealerTrigger(ai); }
            static Trigger* summon_water_elemental(PlayerbotAI* ai) { return new SummonWaterElementalTrigger(ai); }
            static Trigger* slow(PlayerbotAI* ai) { return new SlowTrigger(ai); }
        };
    };
};


namespace ai
{
    namespace mage
    {
        using namespace ai;

        class AiObjectContextInternal : public NamedObjectContext<Action>
        {
        public:
            AiObjectContextInternal()
            {
                creators["frostbolt"] = &AiObjectContextInternal::frostbolt;
                creators["blizzard"] = &AiObjectContextInternal::blizzard;
                creators["frost nova"] = &AiObjectContextInternal::frost_nova;
                creators["arcane intellect"] = &AiObjectContextInternal::arcane_intellect;
                creators["arcane intellect on party"] = &AiObjectContextInternal::arcane_intellect_on_party;
                creators["conjure water"] = &AiObjectContextInternal::conjure_water;
                creators["conjure food"] = &AiObjectContextInternal::conjure_food;
                creators["molten armor"] = &AiObjectContextInternal::molten_armor;
                creators["mage armor"] = &AiObjectContextInternal::mage_armor;
                creators["ice armor"] = &AiObjectContextInternal::ice_armor;
                creators["frost armor"] = &AiObjectContextInternal::frost_armor;
                creators["fireball"] = &AiObjectContextInternal::fireball;
                creators["pyroblast"] = &AiObjectContextInternal::pyroblast;
                creators["flamestrike"] = &AiObjectContextInternal::flamestrike;
                creators["fire blast"] = &AiObjectContextInternal::fire_blast;
                creators["scorch"] = &AiObjectContextInternal::scorch;
                creators["counterspell"] = &AiObjectContextInternal::counterspell;
                creators["remove curse"] = &AiObjectContextInternal::remove_curse;
                creators["remove curse on party"] = &AiObjectContextInternal::remove_curse_on_party;
                creators["icy veins"] = &AiObjectContextInternal::icy_veins;
                creators["combustion"] = &AiObjectContextInternal::combustion;
                creators["ice block"] = &AiObjectContextInternal::ice_block;
                creators["polymorph"] = &AiObjectContextInternal::polymorph;
                creators["polymorph on cc"] = &AiObjectContextInternal::polymorph_on_cc;
                creators["spellsteal"] = &AiObjectContextInternal::spellsteal;
                creators["living bomb"] = &AiObjectContextInternal::living_bomb;
                creators["dragon's breath"] = &AiObjectContextInternal::dragons_breath;
                creators["blast wave"] = &AiObjectContextInternal::blast_wave;
                creators["invisibility"] = &AiObjectContextInternal::invisibility;
                creators["evocation"] = &AiObjectContextInternal::evocation;
                creators["arcane blast"] = &AiObjectContextInternal::arcane_blast;
                creators["arcane barrage"] = &AiObjectContextInternal::arcane_barrage;
                creators["arcane missiles"] = &AiObjectContextInternal::arcane_missiles;
                creators["counterspell on enemy healer"] = &AiObjectContextInternal::counterspell_on_enemy_healer;
                creators["cold snap"] = &AiObjectContextInternal::cold_snap;
                creators["ice barrier"] = &AiObjectContextInternal::ice_barrier;
                creators["summon water elemental"] = &AiObjectContextInternal::summon_water_elemental;
                creators["frostfire bolt"] = &AiObjectContextInternal::frostfire_bolt;
                creators["ice lance"] = &AiObjectContextInternal::ice_lance;
                creators["blink"] = &AiObjectContextInternal::blink;
                creators["cone of cold"] = &AiObjectContextInternal::cone_of_cold;
                creators["arcane explosion"] = &AiObjectContextInternal::arcane_explosion;
                creators["mana shield"] = &AiObjectContextInternal::mana_shield;
                creators["presence of mind"] = &AiObjectContextInternal::presence_of_mind;
                creators["arcane power"] = &AiObjectContextInternal::arcane_power;
                creators["slow"] = &AiObjectContextInternal::slow;
                creators["fire ward"] = &AiObjectContextInternal::fire_ward;
                creators["frost ward"] = &AiObjectContextInternal::frost_ward;
                creators["boost"] = &AiObjectContextInternal::mage_boost;
                creators["instant action"] = &AiObjectContextInternal::ice_lance;
            }
            ~AiObjectContextInternal()
            {
                creators.erase("frostbolt");
                creators.erase("blizzard");
                creators.erase("frost nova");
                creators.erase("arcane intellect");
                creators.erase("arcane intellect on party");
                creators.erase("conjure water");
                creators.erase("conjure food");
                creators.erase("molten armor");
                creators.erase("mage armor");
                creators.erase("ice armor");
                creators.erase("frost armor");
                creators.erase("fireball");
                creators.erase("pyroblast");
                creators.erase("flamestrike");
                creators.erase("fire blast");
                creators.erase("scorch");
                creators.erase("counterspell");
                creators.erase("remove curse");
                creators.erase("remove curse on party");
                creators.erase("icy veins");
                creators.erase("combustion");
                creators.erase("ice block");
                creators.erase("polymorph");
                creators.erase("polymorph on cc");
                creators.erase("spellsteal");
                creators.erase("living bomb");
                creators.erase("dragon's breath");
                creators.erase("blast wave");
                creators.erase("invisibility");
                creators.erase("evocation");
                creators.erase("arcane blast");
                creators.erase("arcane barrage");
                creators.erase("arcane missiles");
                creators.erase("counterspell on enemy healer");
                creators.erase("cold snap");
                creators.erase("ice barrier");
                creators.erase("summon water elemental");
                creators.erase("frostfire bolt");
                creators.erase("ice lance");
                creators.erase("blink");
                creators.erase("cone of cold");
                creators.erase("arcane explosion");
                creators.erase("mana shield");
                creators.erase("presence of mind");
                creators.erase("arcane power");
                creators.erase("slow");
                creators.erase("fire ward");
                creators.erase("frost ward");
                creators.erase("boost");
            }

        private:
            static Action* arcane_missiles(PlayerbotAI* ai) { return new CastArcaneMissilesAction(ai); }
            static Action* arcane_barrage(PlayerbotAI* ai) { return new CastArcaneBarrageAction(ai); }
            static Action* arcane_blast(PlayerbotAI* ai) { return new CastArcaneBlastAction(ai); }
            static Action* frostbolt(PlayerbotAI* ai) { return new CastFrostboltAction(ai); }
            static Action* blizzard(PlayerbotAI* ai) { return new CastBlizzardAction(ai); }
            static Action* frost_nova(PlayerbotAI* ai) { return new CastFrostNovaAction(ai); }
            static Action* arcane_intellect(PlayerbotAI* ai) { return new CastArcaneIntellectAction(ai); }
            static Action* arcane_intellect_on_party(PlayerbotAI* ai) { return new CastArcaneIntellectOnPartyAction(ai); }
            static Action* conjure_water(PlayerbotAI* ai) { return new CastConjureWaterAction(ai); }
            static Action* conjure_food(PlayerbotAI* ai) { return new CastConjureFoodAction(ai); }
            static Action* molten_armor(PlayerbotAI* ai) { return new CastMoltenArmorAction(ai); }
            static Action* mage_armor(PlayerbotAI* ai) { return new CastMageArmorAction(ai); }
            static Action* ice_armor(PlayerbotAI* ai) { return new CastIceArmorAction(ai); }
            static Action* frost_armor(PlayerbotAI* ai) { return new CastFrostArmorAction(ai); }
            static Action* fireball(PlayerbotAI* ai) { return new CastFireballAction(ai); }
            static Action* pyroblast(PlayerbotAI* ai) { return new CastPyroblastAction(ai); }
            static Action* flamestrike(PlayerbotAI* ai) { return new CastFlamestrikeAction(ai); }
            static Action* fire_blast(PlayerbotAI* ai) { return new CastFireBlastAction(ai); }
            static Action* scorch(PlayerbotAI* ai) { return new CastScorchAction(ai); }
            static Action* counterspell(PlayerbotAI* ai) { return new CastCounterspellAction(ai); }
            static Action* remove_curse(PlayerbotAI* ai) { return new CastRemoveCurseAction(ai); }
            static Action* remove_curse_on_party(PlayerbotAI* ai) { return new CastRemoveCurseOnPartyAction(ai); }
            static Action* icy_veins(PlayerbotAI* ai) { return new CastIcyVeinsAction(ai); }
            static Action* combustion(PlayerbotAI* ai) { return new CastCombustionAction(ai); }
            static Action* ice_block(PlayerbotAI* ai) { return new CastIceBlockAction(ai); }
            static Action* polymorph(PlayerbotAI* ai) { return new CastPolymorphAction(ai); }
            static Action* polymorph_on_cc(PlayerbotAI* ai) { return new CastPolymorphCcAction(ai); }
            static Action* spellsteal(PlayerbotAI* ai) { return new CastSpellstealAction(ai); }
            static Action* living_bomb(PlayerbotAI* ai) { return new CastLivingBombAction(ai); }
            static Action* dragons_breath(PlayerbotAI* ai) { return new CastDragonsBreathAction(ai); }
            static Action* blast_wave(PlayerbotAI* ai) { return new CastBlastWaveAction(ai); }
            static Action* invisibility(PlayerbotAI* ai) { return new CastInvisibilityAction(ai); }
            static Action* evocation(PlayerbotAI* ai) { return new CastEvocationAction(ai); }
            static Action* counterspell_on_enemy_healer(PlayerbotAI* ai) { return new CastCounterspellOnEnemyHealerAction(ai); }
            static Action* cold_snap(PlayerbotAI* ai) { return new CastColdSnapAction(ai); }
            static Action* ice_barrier(PlayerbotAI* ai) { return new CastIceBarrierAction(ai); }
            static Action* summon_water_elemental(PlayerbotAI* ai) { return new CastSummonWaterElementalAction(ai); }
            static Action* frostfire_bolt(PlayerbotAI* ai) { return new CastFrostfireBoltAction(ai); }
            static Action* ice_lance(PlayerbotAI* ai) { return new CastIceLanceAction(ai); }
            static Action* blink(PlayerbotAI* ai) { return new CastBlinkAction(ai); }
            static Action* cone_of_cold(PlayerbotAI* ai) { return new CastConeOfColdAction(ai); }
            static Action* arcane_explosion(PlayerbotAI* ai) { return new CastArcaneExplosionAction(ai); }
            static Action* mana_shield(PlayerbotAI* ai) { return new CastManaShieldAction(ai); }
            static Action* presence_of_mind(PlayerbotAI* ai) { return new CastPresenceMindAction(ai); }
            static Action* arcane_power(PlayerbotAI* ai) { return new CastArcanePowerAction(ai); }
            static Action* slow(PlayerbotAI* ai) { return new CastSlowAction(ai); }
            static Action* fire_ward(PlayerbotAI* ai) { return new CastFireWardAction(ai); }
            static Action* frost_ward(PlayerbotAI* ai) { return new CastFrostWardAction(ai); }
            static Action* mage_boost(PlayerbotAI* ai) { return new CastIcyVeinsAction(ai); }
        };
    };
};



MageAiObjectContext::MageAiObjectContext(PlayerbotAI* ai) : AiObjectContext(ai)
{
    strategyFactoryInternal = new ai::mage::StrategyFactoryInternal();
    mageStrategyFactoryInternal = new ai::mage::MageStrategyFactoryInternal();
    buffStrategyFactoryInternal = new ai::mage::MageBuffStrategyFactoryInternal();
    aiObjectContextInternal = new ai::mage::AiObjectContextInternal();
    triggerFactoryInternal = new ai::mage::TriggerFactoryInternal();

    strategyContexts.Add(strategyFactoryInternal);
    strategyContexts.Add(mageStrategyFactoryInternal);
    strategyContexts.Add(buffStrategyFactoryInternal);
    actionContexts.Add(aiObjectContextInternal);
    triggerContexts.Add(triggerFactoryInternal);
}

MageAiObjectContext::~MageAiObjectContext()
{
    strategyContexts.Remove(strategyFactoryInternal);
    strategyContexts.Remove(mageStrategyFactoryInternal);
    strategyContexts.Remove(buffStrategyFactoryInternal);
    actionContexts.Remove(aiObjectContextInternal);
    triggerContexts.Remove(triggerFactoryInternal);

    delete dynamic_cast<ai::mage::StrategyFactoryInternal*>(strategyFactoryInternal);
    delete dynamic_cast<ai::mage::MageStrategyFactoryInternal*>(mageStrategyFactoryInternal);
    delete dynamic_cast<ai::mage::MageBuffStrategyFactoryInternal*>(buffStrategyFactoryInternal);
    delete dynamic_cast<ai::mage::AiObjectContextInternal*>(aiObjectContextInternal);
    delete dynamic_cast<ai::mage::TriggerFactoryInternal*>(triggerFactoryInternal);
}

