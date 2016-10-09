/** @file msetcmp.cc
 * @brief MSetItem comparison functions and functors.
 */
/* Copyright (C) 2006,2009,2013 Olly Betts
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301
 * USA
 */

#include <config.h>
#include "msetcmp.h"

/* We use templates to generate the 14 different comparison functions
 * which we need.  This avoids having to write them all out by hand.
 */

// Order by did.  Helper comparison template function, which is used as the
// last fallback by the others.
template<bool FORWARD_DID, bool CHECK_DID_ZERO> inline bool
msetcmp_by_did(const Xapian::Internal::MSetItem &a,
	       const Xapian::Internal::MSetItem &b)
{
    if (FORWARD_DID) {
	if (CHECK_DID_ZERO) {
	    // We want dummy did 0 to compare worse than any other.
	    if (a.did == 0) return false;
	    if (b.did == 0) return true;
	}
	return (a.did < b.did);
    } else {
	return (a.did > b.did);
    }
}

// Order by relevance, then docid.
template<bool FORWARD_DID> bool
msetcmp_by_relevance(const Xapian::Internal::MSetItem &a,
		     const Xapian::Internal::MSetItem &b)
{
    if (a.wt > b.wt) return true;
    if (a.wt < b.wt) return false;
    return msetcmp_by_did<FORWARD_DID, true>(a, b);
}

// Order by value, then docid.
template<bool FORWARD_VALUE, bool FORWARD_DID, bool DRIECT_NULL_VALUE> bool
msetcmp_by_value(const Xapian::Internal::MSetItem &a,
		 const Xapian::Internal::MSetItem &b)
{
    if (!FORWARD_VALUE) {
	// We want dummy did 0 to compare worse than any other.
	if (a.did == 0) return false;
	if (b.did == 0) return true;
    }
	if (DRIECT_NULL_VALUE) {
		if (a.sort_key.empty()) {
			return true;
		}
		else if (b.sort_key.empty()) {
			return false;
		}
		else {
			if (a.sort_key > b.sort_key) return FORWARD_VALUE;
			if (a.sort_key < b.sort_key) return !FORWARD_VALUE;
		}
	}
	else {
		if (a.sort_key > b.sort_key) return FORWARD_VALUE;
		if (a.sort_key < b.sort_key) return !FORWARD_VALUE;
	}
    return msetcmp_by_did<FORWARD_DID, FORWARD_VALUE>(a, b);
}

//add zkb : Order null value
// Order by value, then relevance, then docid.
template<bool FORWARD_VALUE, bool FORWARD_DID, bool DRIECT_NULL_VALUE> bool
msetcmp_by_value_then_relevance(const Xapian::Internal::MSetItem &a,
				const Xapian::Internal::MSetItem &b)
{
    if (!FORWARD_VALUE) {
	// two special cases to make min_item compares work when did == 0
	if (a.did == 0) return false;
	if (b.did == 0) return true;
    }
	if (DRIECT_NULL_VALUE) {
		if (a.sort_key.empty()) {
			return true;
		}
		else if (b.sort_key.empty()) {
			return false;
		}
		else {
			if (a.sort_key > b.sort_key) return FORWARD_VALUE;
			if (a.sort_key < b.sort_key) return !FORWARD_VALUE;
		}
	}
	else {
		if (a.sort_key > b.sort_key) return FORWARD_VALUE;
		if (a.sort_key < b.sort_key) return !FORWARD_VALUE;
	}
    if (a.wt > b.wt) return true;
    if (a.wt < b.wt) return false;
    return msetcmp_by_did<FORWARD_DID, FORWARD_VALUE>(a, b);
}

//add zkb : Order null value
// Order by relevance, then value, then docid.
template<bool FORWARD_VALUE, bool FORWARD_DID, bool DRIECT_NULL_VALUE> bool
msetcmp_by_relevance_then_value(const Xapian::Internal::MSetItem &a,
				const Xapian::Internal::MSetItem &b)
{
    if (!FORWARD_VALUE) {
	// two special cases to make min_item compares work when did == 0
	if (a.did == 0) return false;
	if (b.did == 0) return true;
    }
    if (a.wt > b.wt) return true;
    if (a.wt < b.wt) return false;
	if (DRIECT_NULL_VALUE) {
		if (a.sort_key.empty()) {
			return true;
		}
		else if (b.sort_key.empty()) {
			return false;
		}
		else {
			if (a.sort_key > b.sort_key) return FORWARD_VALUE;
			if (a.sort_key < b.sort_key) return !FORWARD_VALUE;
		}
	}
	else {
		if (a.sort_key > b.sort_key) return FORWARD_VALUE;
		if (a.sort_key < b.sort_key) return !FORWARD_VALUE;
	}
    return msetcmp_by_did<FORWARD_DID, FORWARD_VALUE>(a, b);
}

static mset_cmp mset_cmp_table[][16] = { {
	// Xapian::Enquire::Internal::REL
	msetcmp_by_relevance<false>,
	0,
	msetcmp_by_relevance<true>,
	0,
	// Xapian::Enquire::Internal::VAL
	msetcmp_by_value<false, false, false>,
	msetcmp_by_value<true, false, false>,
	msetcmp_by_value<false, true, false>,
	msetcmp_by_value<true, true, false>,
	// Xapian::Enquire::Internal::VAL_REL
	msetcmp_by_value_then_relevance<false, false, false>,
	msetcmp_by_value_then_relevance<true, false, false>,
	msetcmp_by_value_then_relevance<false, true, false>,
	msetcmp_by_value_then_relevance<true, true, false>,
	// Xapian::Enquire::Internal::REL_VAL
	msetcmp_by_relevance_then_value<false, false, false>,
	msetcmp_by_relevance_then_value<true, false, false>,
	msetcmp_by_relevance_then_value<false, true, false>,
	msetcmp_by_relevance_then_value<true, true, false>
}, {
	// Xapian::Enquire::Internal::REL
	msetcmp_by_relevance<false>,
	0,
	msetcmp_by_relevance<true>,
	0,
	// Xapian::Enquire::Internal::VAL
	msetcmp_by_value<false, false, true>,
	msetcmp_by_value<true, false, true>,
	msetcmp_by_value<false, true, true>,
	msetcmp_by_value<true, true, true>,
	// Xapian::Enquire::Internal::VAL_REL
	msetcmp_by_value_then_relevance<false, false, true>,
	msetcmp_by_value_then_relevance<true, false, true>,
	msetcmp_by_value_then_relevance<false, true, true>,
	msetcmp_by_value_then_relevance<true, true, true>,
	// Xapian::Enquire::Internal::REL_VAL
	msetcmp_by_relevance_then_value<false, false, true>,
	msetcmp_by_relevance_then_value<true, false, true>,
	msetcmp_by_relevance_then_value<false, true, true>,
	msetcmp_by_relevance_then_value<true, true, true>
} };

mset_cmp get_msetcmp_function(Xapian::Enquire::Internal::sort_setting sort_by, bool sort_forward, bool sort_value_forward, const bool direct_null) {
    if (sort_by == Xapian::Enquire::Internal::REL) sort_value_forward = false;
	int direct_num = direct_null ? 1 : 0;
    return mset_cmp_table[direct_num][sort_by * 4 + sort_forward * 2 + sort_value_forward];
}
