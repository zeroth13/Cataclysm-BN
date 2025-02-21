#pragma once
#ifndef CATA_SRC_ACTIVITY_ACTOR_H
#define CATA_SRC_ACTIVITY_ACTOR_H

#include <memory>
#include <unordered_map>
#include <vector>

#include "clone_ptr.h"
#include "item_location.h"
#include "optional.h"
#include "pickup_token.h"
#include "point.h"
#include "type_id.h"

class Character;
class JsonIn;
class JsonOut;
class player_activity;

class activity_actor
{
    private:
        /**
         * Returns true if `this` activity is resumable, and `this` and @p other
         * are "equivalent" i.e. similar enough that `this` activity
         * can be resumed instead of starting @p other.
         * Many activities are not resumable, so the default is returning
         * false.
         * @pre @p other is the same type of actor as `this`
         */
        virtual bool can_resume_with_internal( const activity_actor &,
                                               const Character & ) const {
            return false;
        }

    public:
        virtual ~activity_actor() = default;

        /**
         * Should return the activity id of the corresponding activity
         */
        virtual activity_id get_type() const = 0;

        /**
         * Called once at the start of the activity.
         * This may be used to preform setup actions and/or set
         * player_activity::moves_left/moves_total.
         */
        virtual void start( player_activity &act, Character &who ) = 0;

        /**
         * Called on every turn of the activity
         * It may be used to stop the activity prematurely by setting it to null.
         */
        virtual void do_turn( player_activity &act, Character &who ) = 0;

        /**
         * Called when the activity runs out of moves, assuming that it has not
         * already been set to null
         */
        virtual void finish( player_activity &act, Character &who ) = 0;

        /**
         * Called just before Character::cancel_activity() executes.
         * This may be used to perform cleanup
         */
        virtual void canceled( player_activity &/*act*/, Character &/*who*/ ) {};

        /**
         * Called in player_activity::can_resume_with
         * which allows suspended activities to be resumed instead of
         * starting a new activity in certain cases.
         * Checks that @p other has the same type as `this` so that
         * `can_resume_with_internal` can safely `static_cast` @p other.
         */
        bool can_resume_with( const activity_actor &other, const Character &who ) const {
            if( other.get_type() == get_type() ) {
                return can_resume_with_internal( other, who );
            }

            return false;
        }

        /**
         * Returns a deep copy of this object. Example implementation:
         * \code
         * class my_activity_actor {
         *     std::unique_ptr<activity_actor> clone() const override {
         *         return std::make_unique<my_activity_actor>( *this );
         *     }
         * };
         * \endcode
         * The returned value should behave like the original item and must have the same type.
         */
        virtual std::unique_ptr<activity_actor> clone() const = 0;

        /**
         * Must write any custom members of the derived class to json
         * Note that a static member function for deserialization must also be created and
         * added to the `activity_actor_deserializers` hashmap in activity_actor.cpp
         */
        virtual void serialize( JsonOut &jsout ) const = 0;
};

class dig_activity_actor : public activity_actor
{
    private:
        int moves_total;
        /** location of the dig **/
        tripoint location;
        std::string result_terrain;
        tripoint byproducts_location;
        int byproducts_count;
        std::string byproducts_item_group;

        /**
         * Returns true if @p other and `this` are "equivalent" in the sense that
         *  `this` can be resumed instead of starting @p other.
         */
        bool equivalent_activity( const dig_activity_actor &other ) const {
            return  location == other.location &&
                    result_terrain == other.result_terrain &&
                    byproducts_location == other.byproducts_location &&
                    byproducts_count == other.byproducts_count &&
                    byproducts_item_group == other.byproducts_item_group;
        }

        /**
         * @pre @p other is a `dig_activity_actor`
         */
        bool can_resume_with_internal( const activity_actor &other, const Character & ) const override {
            const dig_activity_actor &d_actor = static_cast<const dig_activity_actor &>( other );
            return equivalent_activity( d_actor );
        }

    public:
        dig_activity_actor(
            int dig_moves, const tripoint &dig_loc,
            const std::string &resulting_ter, const tripoint &dump_loc,
            int dump_spawn_count, const std::string &dump_item_group
        ):
            moves_total( dig_moves ), location( dig_loc ),
            result_terrain( resulting_ter ),
            byproducts_location( dump_loc ),
            byproducts_count( dump_spawn_count ),
            byproducts_item_group( dump_item_group ) {}

        activity_id get_type() const override {
            return activity_id( "ACT_DIG" );
        }

        void start( player_activity &act, Character & ) override;
        void do_turn( player_activity &, Character & ) override;
        void finish( player_activity &act, Character &who ) override;

        std::unique_ptr<activity_actor> clone() const override {
            return std::make_unique<dig_activity_actor>( *this );
        }

        void serialize( JsonOut &jsout ) const override;
        static std::unique_ptr<activity_actor> deserialize( JsonIn &jsin );
};

class dig_channel_activity_actor : public activity_actor
{
    private:
        int moves_total;
        /** location of the dig **/
        tripoint location;
        std::string result_terrain;
        tripoint byproducts_location;
        int byproducts_count;
        std::string byproducts_item_group;

        /**
         * Returns true if @p other and `this` are "equivalent" in the sense that
         *  `this` can be resumed instead of starting @p other.
         */
        bool equivalent_activity( const dig_channel_activity_actor &other ) const {
            return  location == other.location &&
                    result_terrain == other.result_terrain &&
                    byproducts_location == other.byproducts_location &&
                    byproducts_count == other.byproducts_count &&
                    byproducts_item_group == other.byproducts_item_group;
        }

        /**
         * @pre @p other is a `dig_activity_actor`
         */
        bool can_resume_with_internal( const activity_actor &other, const Character & ) const override {
            const dig_channel_activity_actor &dc_actor = static_cast<const dig_channel_activity_actor &>
                    ( other );
            return equivalent_activity( dc_actor );
        }

    public:
        dig_channel_activity_actor(
            int dig_moves, const tripoint &dig_loc,
            const std::string &resulting_ter, const tripoint &dump_loc,
            int dump_spawn_count, const std::string &dump_item_group
        ):
            moves_total( dig_moves ), location( dig_loc ),
            result_terrain( resulting_ter ),
            byproducts_location( dump_loc ),
            byproducts_count( dump_spawn_count ),
            byproducts_item_group( dump_item_group ) {}

        activity_id get_type() const override {
            return activity_id( "ACT_DIG_CHANNEL" );
        }

        void start( player_activity &act, Character & ) override;
        void do_turn( player_activity &, Character & ) override;
        void finish( player_activity &act, Character &who ) override;

        std::unique_ptr<activity_actor> clone() const override {
            return std::make_unique<dig_channel_activity_actor>( *this );
        }

        void serialize( JsonOut &jsout ) const override;
        static std::unique_ptr<activity_actor> deserialize( JsonIn &jsin );
};

class hacking_activity_actor : public activity_actor
{
    private:
        bool using_bionic = false;

    public:
        struct use_bionic {};

        hacking_activity_actor() = default;
        hacking_activity_actor( use_bionic );

        activity_id get_type() const override {
            return activity_id( "ACT_HACKING" );
        }

        void start( player_activity &act, Character &who ) override;
        void do_turn( player_activity &, Character & ) override {};
        void finish( player_activity &act, Character &who ) override;

        std::unique_ptr<activity_actor> clone() const override {
            return std::make_unique<hacking_activity_actor>( *this );
        }

        void serialize( JsonOut &jsout ) const override;
        static std::unique_ptr<activity_actor> deserialize( JsonIn &jsin );
};

class move_items_activity_actor : public activity_actor
{
    private:
        std::vector<item_location> target_items;
        std::vector<int> quantities;
        bool to_vehicle;
        tripoint relative_destination;

    public:
        move_items_activity_actor( std::vector<item_location> target_items, std::vector<int> quantities,
                                   bool to_vehicle, tripoint relative_destination ) :
            target_items( target_items ), quantities( quantities ), to_vehicle( to_vehicle ),
            relative_destination( relative_destination ) {}

        activity_id get_type() const override {
            return activity_id( "ACT_MOVE_ITEMS" );
        }

        void start( player_activity &, Character & ) override {};
        void do_turn( player_activity &act, Character &who ) override;
        void finish( player_activity &, Character & ) override {};

        std::unique_ptr<activity_actor> clone() const override {
            return std::make_unique<move_items_activity_actor>( *this );
        }

        void serialize( JsonOut &jsout ) const override;
        static std::unique_ptr<activity_actor> deserialize( JsonIn &jsin );
};

class pickup_activity_actor : public activity_actor
{
    private:
        /** Target items and the quantities thereof */
        std::vector<pickup::pick_drop_selection> target_items;

        /**
         * Position of the character when the activity is started. This is
         * stored so that we can cancel the activity if the player moves
         * (e.g. if the player is in a moving vehicle). This should be null
         * if not grabbing from the ground.
         */
        cata::optional<tripoint> starting_pos;

    public:
        pickup_activity_actor( const std::vector<pickup::pick_drop_selection> &target_items,
                               const cata::optional<tripoint> &starting_pos )
            : target_items( target_items )
            , starting_pos( starting_pos ) {}

        activity_id get_type() const override {
            return activity_id( "ACT_PICKUP" );
        }

        void start( player_activity &, Character & ) override {};
        void do_turn( player_activity &act, Character &who ) override;
        void finish( player_activity &, Character & ) override {};

        std::unique_ptr<activity_actor> clone() const override {
            return std::make_unique<pickup_activity_actor>( *this );
        }

        void serialize( JsonOut &jsout ) const override;
        static std::unique_ptr<activity_actor> deserialize( JsonIn &jsin );
};

class migration_cancel_activity_actor : public activity_actor
{
    public:
        migration_cancel_activity_actor() = default;

        activity_id get_type() const override {
            return activity_id( "ACT_MIGRATION_CANCEL" );
        }

        void start( player_activity &, Character & ) override {};
        void do_turn( player_activity &act, Character &who ) override;
        void finish( player_activity &, Character & ) override {};

        std::unique_ptr<activity_actor> clone() const override {
            return std::make_unique<migration_cancel_activity_actor>( *this );
        }

        void serialize( JsonOut &jsout ) const override;
        static std::unique_ptr<activity_actor> deserialize( JsonIn &jsin );
};

class open_gate_activity_actor : public activity_actor
{
    private:
        int moves_total;
        tripoint placement;

        /**
         * @pre @p other is a open_gate_activity_actor
         */
        bool can_resume_with_internal( const activity_actor &other, const Character & ) const override {
            const open_gate_activity_actor &og_actor = static_cast<const open_gate_activity_actor &>( other );
            return placement == og_actor.placement;
        }

    public:
        open_gate_activity_actor( int gate_moves, const tripoint &gate_placement ) :
            moves_total( gate_moves ), placement( gate_placement ) {}

        activity_id get_type() const override {
            return activity_id( "ACT_OPEN_GATE" );
        }

        void start( player_activity &act, Character & ) override;
        void do_turn( player_activity &, Character & ) override {};
        void finish( player_activity &act, Character & ) override;

        std::unique_ptr<activity_actor> clone() const override {
            return std::make_unique<open_gate_activity_actor>( *this );
        }

        void serialize( JsonOut &jsout ) const override;
        static std::unique_ptr<activity_actor> deserialize( JsonIn &jsin );
};

namespace activity_actors
{

// defined in activity_actor.cpp
extern const std::unordered_map<activity_id, std::unique_ptr<activity_actor>( * )( JsonIn & )>
deserialize_functions;

} // namespace activity_actors

void serialize( const cata::clone_ptr<activity_actor> &actor, JsonOut &jsout );
void deserialize( cata::clone_ptr<activity_actor> &actor, JsonIn &jsin );

#endif // CATA_SRC_ACTIVITY_ACTOR_H
