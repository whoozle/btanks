#!/usr/bin/python

"""
/* sl08 - small slot/signals library
 * Copyright (C) 2007-2008 Netive Media Group
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
*/
"""

class Generator(object):
	def __init__(self, n):
		self.__n = n;
		
	def prototype(self, proto):	
		r = '('
		for i in xrange(0, self.__n): 
			if proto: 
				r = r + "arg%d_type " %(i + 1)
			r = r + "a%d" %(i + 1)
			if i + 1 < self.__n:
				r += ', '
		return r + ')'
	
	def template_declaration(self, cname, void = False): 
		if void: 
			r = '<'
		else:
			r = '<typename return_type'
		
		if cname == 'base-signal': 
			for i in xrange(0, self.__n): 
				if not void or i > 0:
					r = r + ", "
				r = r + "typename arg%d_type" %(i + 1)
			r = r + ">"
			return r
		elif cname == 'signal': 
			for i in xrange(0, self.__n): 
				if not void or i > 0:
					r = r + ", "
				r = r + "typename arg%d_type" %(i + 1)
			if not void or self.__n > 0:
				r = r + ", "

			if void: 
				r = r + "class validator_type >"
			else: 
				r = r + "class validator_type = default_validator<return_type> >"
			return r
		elif cname == 'slot': 
			for i in xrange(0, self.__n): 
				if not void or i > 0:
					r = r + ", "
				r = r + "typename arg%d_type" %(i + 1)
			if not void or self.__n > 0:
				r = r + ", "
			r = r + "class object_type>"
			return r
		elif cname == 'base-slot': 
			for i in xrange(0, self.__n): 
				if not void or i > 0:
					r = r + ", "
				r = r + "typename arg%d_type" %(i + 1)
			r = r + ">"
			return r
		
		raise Exception("no class %s defined" % cname)

	def template_parameters(self, cname, void = False): 
		if void: 
			r = '<void'
		else: 
			r = '<return_type'
		
		if cname == 'base-signal': 
			for i in xrange(0, self.__n): 
				r = r + ", arg%d_type" %(i + 1)
			r = r + ">"
			return r
		elif cname == 'signal': 
			for i in xrange(0, self.__n): 
				r = r + ", arg%d_type" %(i + 1)
			r = r + ", validator_type>"
			return r
		elif cname == 'slot': 
			for i in xrange(0, self.__n): 
				r = r + ", arg%d_type" %(i + 1)
			r = r + ", object_type>"
			return r
		elif cname == 'base-slot': 
			for i in xrange(0, self.__n): 
				r = r + ", arg%d_type" %(i + 1)
			r = r + ">"
			return r
		
		raise Exception("no class %s defined" % cname)

	def generate(self): 
		text = """
		template %s class base_signalXXX;

		template %s 
		class base_slotXXX {
			typedef base_signalXXX %s signal_type; 
			typedef std::list<signal_type *> signals_type;
			signals_type signals;
		public: 
			virtual return_type operator() %s const = 0;
			inline base_slotXXX () : signals() {} 

			inline void connect(signal_type &signal_ref) {
				signal_type *signal = &signal_ref;
				signals.push_back(signal);
				signal->connect(this); 
			}

			inline void _disconnect(signal_type *signal) {
				for(typename signals_type::iterator i = signals.begin(); i != signals.end(); ) {
					if (*i == signal) {
						i = signals.erase(i);
					} else ++i;
				}
			}
		
			inline void disconnect() {
				for(typename signals_type::iterator i = signals.begin(); i != signals.end(); ++i) {
					(*i)->_disconnect(this); 
				}
				signals.clear();
			} 
			inline virtual ~base_slotXXX() { 
				disconnect();
			}
		};

		template %s
		class slotXXX : public base_slotXXX %s { 
		public: 
			typedef base_signalXXX %s signal_type; 
			typedef return_type (object_type::*func_t) %s; 

			inline slotXXX () : object(NULL), func(NULL) {}
			inline slotXXX(object_type *object, func_t func, signal_type * signal = NULL) : object(object), func(func) {}

			inline void assign(object_type *o, func_t f) { object = o; func = f; }
			inline void assign(object_type *o, func_t f, signal_type &signal_ref = NULL) { object = o; func = f; connect(signal_ref); }
	
			inline return_type operator() %s const { 
				return (object->*func) %s ;
			} 
	
		private: 
			object_type *object; 
			func_t func;
		}; 

		""" %(
			self.template_declaration('base-signal'), 
			self.template_declaration('base-slot'), 
			self.template_parameters('base-signal'), 
			self.prototype(True),
			self.template_declaration('slot'), 
			self.template_parameters('base-slot'), 
			self.template_parameters('base-slot'), 
			self.prototype(True), self.prototype(True), self.prototype(False),
			)
			
		#specialized version of the slot
			
		text = text + """
		template %s
		class slotXXX %s : public base_slotXXX %s{
		public: 
			typedef base_signalXXX %s signal_type; 
			typedef void (object_type::*func_t) %s ;

			inline slotXXX () : object(NULL), func(NULL) {}
			inline slotXXX (object_type *object, func_t func) : object(object), func(func) {}
			inline void assign(object_type *o, func_t f) { object = o; func = f; }
			inline void assign(object_type *o, func_t f, signal_type &signal_ref) { object = o; func = f; connect(signal_ref); }
	
			inline void operator() %s const { 
				(object->*func) %s; 
			} 
	
		private: 
			object_type *object; 
			func_t func; 
		};
		""" %(
			self.template_declaration('slot', True), 
			self.template_parameters('slot', True), 
			self.template_parameters('base-slot', True), 
			self.template_parameters('base-slot', True), 
			self.prototype(True), 
			self.prototype(True), 
			self.prototype(False), 
		)
						
		
		text = text + """
		template %s
		class base_signalXXX {
		protected: 
			typedef base_slotXXX %s slot_type; 
			typedef std::list<slot_type *> slots_type;
			slots_type slots;
		
		public: 
			inline virtual return_type emit %s const = 0;

			inline void connect(slot_type *slot) {
				slots.push_back(slot);
			} 

			inline void _disconnect(slot_type *slot) {
				for(typename slots_type::iterator i = slots.begin(); i != slots.end(); ) { 
					if (slot != *i) 
						++i; 
					else 
						i = slots.erase(i); 
				} 
			} 

			inline void disconnect() {
				for(typename slots_type::iterator i = slots.begin(); i != slots.end(); ++i) { 
					(*i)->_disconnect(this);
				} 
				slots.clear();
			}
			inline virtual ~base_signalXXX() {
				disconnect();
			}
		};
		
		template %s
		class signalXXX : public base_signalXXX %s { 
		public: 
			typedef base_signalXXX %s parent_type; 
			typedef typename deconst<return_type>::type non_const_return_type;
			
			inline virtual return_type emit %s const {
				validator_type v;
				non_const_return_type r;
				
				for(typename parent_type::slots_type::const_iterator i = parent_type::slots.begin(); i != parent_type::slots.end(); ++i) { 
					r = (*i)->operator() %s;
					if (!v(r))
						return r;
				}
				return r; 
			} 
		};
			
		"""  %(
			self.template_declaration('base-signal'), 
			self.template_parameters('base-slot'), 
			self.prototype(True),
			self.template_declaration('signal'), 
			self.template_parameters('base-signal'), 
			self.template_parameters('base-signal'), 
			self.prototype(True),
			self.prototype(False),
			)

		text = text + """template %s
		class signalXXX %s : public base_signalXXX %s { 
		typedef base_signalXXX %s parent_type; 
		public: 
			inline void emit %s const {  
				for(typename parent_type::slots_type::const_iterator i = parent_type::slots.begin(); i != parent_type::slots.end(); ++i) { 
					(*i)->operator() %s ; 
				} 
			} 
		};
		""" %(
			self.template_declaration('signal', True), 
			self.template_parameters('signal', True), 
			self.template_parameters('base-signal', True), 
			self.template_parameters('base-signal', True), 
			self.prototype(True), 
			self.prototype(False), 
		)
		
		for x in ['int', 'bool']: 
			text = text + self.specialize_signal(x)
				
		return text.replace('XXX', str(self.__n));
		
	def specialize_signal(self, t):
		#hack hack hack: replace void with our pod type
		return ("""template %s
		class signalXXX %s : public base_signalXXX %s { 
			typedef base_signalXXX %s parent_type; 
		public: 
			inline void emit %s const {  
				validator_type v;
				void r = (void)0;
				
				for(typename parent_type::slots_type::const_iterator i = parent_type::slots.begin(); i != parent_type::slots.end(); ++i) { 
					r = (*i)->operator() %s;
					if (!v(r))
						return r;
				}
				return r;
			}
		};
		"""  %(
			self.template_declaration('signal', True), 
			self.template_parameters('signal', True), 
			self.template_parameters('base-signal', True), 
			self.template_parameters('base-signal', True), 
			self.prototype(True), 
			self.prototype(False), 
		)).replace('void', t)
		

text = ''
for i in xrange(0, 6):
	g = Generator(i)
	text = text + "\n" + g.generate(); 

print """#ifndef BTANKS_SL08_SLOTSANDSIGNALS_H__
#define BTANKS_SL08_SLOTSANDSIGNALS_H__

/* sl08 - small slot/signals library
 * Copyright (C) 2007-2008 Netive Media Group
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
*/

/* DO NOT MODIFY THIS FILE: IT'S AUTOGENERATED */ 

#include <list>

#ifndef NULL
#define NULL            ((void*) 0)
#endif

namespace sl08 {

		template <typename T> struct deconst { typedef T type; };
		template <typename T> struct deconst<const T> { typedef T type; };

		template <typename result_type>
		class default_validator {
		public:
			inline bool operator()(result_type r) {
				return true;
			}
		};

		template <typename result_type>
		class exclusive_validator {
		public:
			inline bool operator()(result_type r) {
				return !r;
			}
		};

	%s
}

#endif
""" %(text);

