/////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2007-2013
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/intrusive for documentation.
//
/////////////////////////////////////////////////////////////////////////////

#ifndef BHO_INTRUSIVE_GENERIC_HOOK_HPP
#define BHO_INTRUSIVE_GENERIC_HOOK_HPP

#ifndef BHO_CONFIG_HPP
#  include <asio2/bho/config.hpp>
#endif

#if defined(BHO_HAS_PRAGMA_ONCE)
#  pragma once
#endif

#include <asio2/bho/intrusive/pointer_traits.hpp>
#include <asio2/bho/intrusive/link_mode.hpp>
#include <asio2/bho/intrusive/detail/mpl.hpp>
#include <asio2/bho/intrusive/detail/assert.hpp>
#include <asio2/bho/intrusive/detail/node_holder.hpp>
#include <asio2/bho/intrusive/detail/algo_type.hpp>
#include <asio2/bho/static_assert.hpp>

namespace bho {
namespace intrusive {

/// @cond

namespace detail {

template <link_mode_type LinkMode>
struct link_dispatch
{};

template<class Hook>
BHO_INTRUSIVE_FORCEINLINE void destructor_impl(Hook &hook, detail::link_dispatch<safe_link>)
{  //If this assertion raises, you might have destroyed an object
   //while it was still inserted in a container that is alive.
   //If so, remove the object from the container before destroying it.
   (void)hook; BHO_INTRUSIVE_SAFE_HOOK_DESTRUCTOR_ASSERT(!hook.is_linked());
}

template<class Hook>
BHO_INTRUSIVE_FORCEINLINE void destructor_impl(Hook &hook, detail::link_dispatch<auto_unlink>)
{  hook.unlink();  }

template<class Hook>
BHO_INTRUSIVE_FORCEINLINE void destructor_impl(Hook &, detail::link_dispatch<normal_link>)
{}

}  //namespace detail {

enum base_hook_type
{  NoBaseHookId
,  ListBaseHookId
,  SlistBaseHookId
,  RbTreeBaseHookId
,  HashBaseHookId
,  AvlTreeBaseHookId
,  BsTreeBaseHookId
,  TreapTreeBaseHookId
,  AnyBaseHookId
};


template <class HookTags, unsigned int>
struct hook_tags_definer{};

template <class HookTags>
struct hook_tags_definer<HookTags, ListBaseHookId>
{  typedef HookTags default_list_hook;  };

template <class HookTags>
struct hook_tags_definer<HookTags, SlistBaseHookId>
{  typedef HookTags default_slist_hook;  };

template <class HookTags>
struct hook_tags_definer<HookTags, RbTreeBaseHookId>
{  typedef HookTags default_rbtree_hook;  };

template <class HookTags>
struct hook_tags_definer<HookTags, HashBaseHookId>
{  typedef HookTags default_hashtable_hook;  };

template <class HookTags>
struct hook_tags_definer<HookTags, AvlTreeBaseHookId>
{  typedef HookTags default_avltree_hook;  };

template <class HookTags>
struct hook_tags_definer<HookTags, BsTreeBaseHookId>
{  typedef HookTags default_bstree_hook;  };

template <class HookTags>
struct hook_tags_definer<HookTags, AnyBaseHookId>
{  typedef HookTags default_any_hook;  };

template
   < class NodeTraits
   , class Tag
   , link_mode_type LinkMode
   , base_hook_type BaseHookType
   >
struct hooktags_impl
{
   static const link_mode_type link_mode = LinkMode;
   typedef Tag tag;
   typedef NodeTraits node_traits;
   static const bool is_base_hook = !detail::is_same<Tag, member_tag>::value;
   static const bool safemode_or_autounlink = is_safe_autounlink<link_mode>::value;
   static const unsigned int type = BaseHookType;
};

/// @endcond

template
   < bho::intrusive::algo_types Algo
   , class NodeTraits
   , class Tag
   , link_mode_type LinkMode
   , base_hook_type BaseHookType
   >
class generic_hook
   /// @cond
   //If the hook is a base hook, derive generic hook from node_holder
   //so that a unique base class is created to convert from the node
   //to the type. This mechanism will be used by bhtraits.
   //
   //If the hook is a member hook, generic hook will directly derive
   //from the hook.
   : public detail::if_c
      < detail::is_same<Tag, member_tag>::value
      , typename NodeTraits::node
      , node_holder<typename NodeTraits::node, Tag, BaseHookType>
      >::type
   //If this is the a default-tagged base hook derive from a class that
   //will define an special internal typedef. Containers will be able to detect this
   //special typedef and obtain generic_hook's internal types in order to deduce
   //value_traits for this hook.
   , public hook_tags_definer
      < generic_hook<Algo, NodeTraits, Tag, LinkMode, BaseHookType>
      , detail::is_same<Tag, dft_tag>::value ? BaseHookType : NoBaseHookId>
   /// @endcond
{
   /// @cond
   typedef typename get_algo<Algo, NodeTraits>::type  node_algorithms;
   typedef typename node_algorithms::node             node;
   typedef typename node_algorithms::node_ptr         node_ptr;
   typedef typename node_algorithms::const_node_ptr   const_node_ptr;

   public:

   typedef hooktags_impl
      < NodeTraits
      , Tag, LinkMode, BaseHookType>                  hooktags;

   BHO_INTRUSIVE_FORCEINLINE node_ptr this_ptr() BHO_NOEXCEPT
   {  return pointer_traits<node_ptr>::pointer_to(static_cast<node&>(*this)); }

   BHO_INTRUSIVE_FORCEINLINE const_node_ptr this_ptr() const BHO_NOEXCEPT
   {  return pointer_traits<const_node_ptr>::pointer_to(static_cast<const node&>(*this)); }

   public:
   /// @endcond

   BHO_INTRUSIVE_FORCEINLINE generic_hook() BHO_NOEXCEPT
   {
      if(hooktags::safemode_or_autounlink){
         node_algorithms::init(this->this_ptr());
      }
   }

   BHO_INTRUSIVE_FORCEINLINE generic_hook(const generic_hook& ) BHO_NOEXCEPT
   {
      if(hooktags::safemode_or_autounlink){
         node_algorithms::init(this->this_ptr());
      }
   }

   BHO_INTRUSIVE_FORCEINLINE generic_hook& operator=(const generic_hook& ) BHO_NOEXCEPT
   {  return *this;  }

   BHO_INTRUSIVE_FORCEINLINE ~generic_hook()
   {
      destructor_impl
         (*this, detail::link_dispatch<hooktags::link_mode>());
   }

   BHO_INTRUSIVE_FORCEINLINE void swap_nodes(generic_hook &other) BHO_NOEXCEPT
   {
      node_algorithms::swap_nodes
         (this->this_ptr(), other.this_ptr());
   }

   BHO_INTRUSIVE_FORCEINLINE bool is_linked() const BHO_NOEXCEPT
   {
      //is_linked() can be only used in safe-mode or auto-unlink
      BHO_STATIC_ASSERT(( hooktags::safemode_or_autounlink ));
      return !node_algorithms::unique(this->this_ptr());
   }

   BHO_INTRUSIVE_FORCEINLINE void unlink() BHO_NOEXCEPT
   {
      BHO_STATIC_ASSERT(( (int)hooktags::link_mode == (int)auto_unlink ));
      node_ptr n(this->this_ptr());
      if(!node_algorithms::inited(n)){
         node_algorithms::unlink(n);
         node_algorithms::init(n);
      }
   }
};

} //namespace intrusive
} //namespace bho

#endif //BHO_INTRUSIVE_GENERIC_HOOK_HPP
