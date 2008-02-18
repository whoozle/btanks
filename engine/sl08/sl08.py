#!/usr/bin/python

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
			signal_type *signal; 
		public: 
			virtual return_type operator() %s = 0;
			inline base_slotXXX () : signal(NULL) {} 
			inline void connect(signal_type &signal_ref) {
				disconnect();
				signal = &signal_ref; 
				signal->connect(this); 
			}
		
			inline void disconnect() {
				if (signal != NULL) {
					signal->disconnect(this); 
					signal = NULL; 	
				}
			} 
			inline virtual ~base_slotXXX() { 
				disconnect();
			}
		};

		template %s
		class slotXXX : public base_slotXXX %s { 
		public: 
			typedef return_type (object_type::*func_t) %s; 
			inline slotXXX(object_type *object, func_t func) : object(object), func(func) {}
	
			inline return_type operator() %s { 
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
			self.prototype(True), self.prototype(True), self.prototype(False),
			)
			
		#specialized version of the slot
			
		text = text + """
		template %s
		class slotXXX %s : public base_slotXXX %s{
		public: 
			typedef void (object_type::*func_t) %s ;
			inline slotXXX (object_type *object, func_t func) : object(object), func(func) {}
	
			inline void operator() %s { 
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
			self.prototype(True), 
			self.prototype(True), 
			self.prototype(False), 
		)
						
		
		text = text + """
		template %s
		class base_signalXXX {
		protected: 
			typedef base_slotXXX %s slot_type; 
			typedef std::deque<slot_type *> slots_type;
			slots_type slots;
		
		public: 
			inline virtual return_type emit %s = 0;

			inline void connect(slot_type *slot) {
				slots.push_back(slot);
			} 

			inline void disconnect(slot_type *slot) {
				for(typename slots_type::iterator i = slots.begin(); i != slots.end(); ) { 
					if (slot != *i) 
						++i; 
					else 
						i = slots.erase(i); 
				} 
			} 
			inline virtual ~base_signalXXX() {}
		};
		
		template %s
		class signalXXX : public base_signalXXX %s { 
		public: 
			typedef base_signalXXX %s parent_type; 
			
			inline virtual return_type emit %s {
				return_type r; 
				for(typename parent_type::slots_type::iterator i = parent_type::slots.begin(); i != parent_type::slots.end(); ++i) { 
					r = (*i)->operator() %s; 
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
			inline void emit %s {  
				for(typename parent_type::slots_type::iterator i = parent_type::slots.begin(); i != parent_type::slots.end(); ++i) { 
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
				
		return text.replace('XXX', str(self.__n));
		

text = ''
for i in xrange(0, 2): 
	g = Generator(i)
	text = text + "\n" + g.generate(); 

print """#ifndef BTANKS_SL08_SLOTSANDSIGNALS_H__
#define BTANKS_SL08_SLOTSANDSIGNALS_H__

#include <deque>

#ifndef NULL
#define NULL            ((void*) 0)
#endif

namespace sl08 {

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
				return (bool)r;
			}
		};

	%s
}

#endif
""" %(text);

