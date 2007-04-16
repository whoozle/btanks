#ifndef __STACKVM_SINGLETON_H__
#define __STACKVM_SINGLETON_H__

#include "mrt/export_mrt.h"

/* M-runtime for c++
 * Copyright (C) 2005-2007 Vladimir Menshakov
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

namespace mrt {
	template <class S> class Accessor {
	public:

		inline S* operator->() const {
			static S * p = S::get_instance();
			return p;
		}

		inline const S* get_const() const {
			static S * p = S::get_instance();
			return p;
		}
	};
}

#define SINGLETON(e, name, class) \
	extern e const mrt::Accessor<class> name

#define DECLARE_SINGLETON(class) \
	static inline class * get_instance() { \
		static class instance; \
		return &instance; \
	} 

#define IMPLEMENT_SINGLETON(name, class) \
	const mrt::Accessor<class> name = mrt::Accessor<class>();


#endif
