#ifndef BTANKS_SL08_SLOTSANDSIGNALS_H__
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

	

		template <typename return_type> class base_signal0;

		template <typename return_type> 
		class base_slot0 {
			typedef base_signal0 <return_type> signal_type; 
			signal_type *signal; 
		public: 
			virtual return_type operator() () = 0;
			inline base_slot0 () : signal(NULL) {} 
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
			inline virtual ~base_slot0() { 
				disconnect();
			}
		};

		template <typename return_type, class object_type>
		class slot0 : public base_slot0 <return_type> { 
		public: 
			typedef return_type (object_type::*func_t) (); 
			inline slot0(object_type *object, func_t func) : object(object), func(func) {}
	
			inline return_type operator() () { 
				return (object->*func) () ;
			} 
	
		private: 
			object_type *object; 
			func_t func;
		}; 

		
		template <class object_type>
		class slot0 <void, object_type> : public base_slot0 <void>{
		public: 
			typedef void (object_type::*func_t) () ;
			inline slot0 (object_type *object, func_t func) : object(object), func(func) {}
	
			inline void operator() () { 
				(object->*func) (); 
			} 
	
		private: 
			object_type *object; 
			func_t func; 
		};
		
		template <typename return_type>
		class base_signal0 {
		protected: 
			typedef base_slot0 <return_type> slot_type; 
			typedef std::deque<slot_type *> slots_type;
			slots_type slots;
		
		public: 
			inline virtual return_type emit () = 0;

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
			inline virtual ~base_signal0() {}
		};
		
		template <typename return_type, class validator_type = default_validator<return_type> >
		class signal0 : public base_signal0 <return_type> { 
		public: 
			typedef base_signal0 <return_type> parent_type; 
			
			inline virtual return_type emit () {
				return_type r; 
				for(typename parent_type::slots_type::iterator i = parent_type::slots.begin(); i != parent_type::slots.end(); ++i) { 
					r = (*i)->operator() (); 
				}
				return r; 
			} 
		};
			
		template <class validator_type >
		class signal0 <void, validator_type> : public base_signal0 <void> { 
		typedef base_signal0 <void> parent_type; 
		public: 
			inline void emit () {  
				for(typename parent_type::slots_type::iterator i = parent_type::slots.begin(); i != parent_type::slots.end(); ++i) { 
					(*i)->operator() () ; 
				} 
			} 
		};
		

		template <typename return_type, typename arg1_type> class base_signal1;

		template <typename return_type, typename arg1_type> 
		class base_slot1 {
			typedef base_signal1 <return_type, arg1_type> signal_type; 
			signal_type *signal; 
		public: 
			virtual return_type operator() (arg1_type a1) = 0;
			inline base_slot1 () : signal(NULL) {} 
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
			inline virtual ~base_slot1() { 
				disconnect();
			}
		};

		template <typename return_type, typename arg1_type, class object_type>
		class slot1 : public base_slot1 <return_type, arg1_type> { 
		public: 
			typedef return_type (object_type::*func_t) (arg1_type a1); 
			inline slot1(object_type *object, func_t func) : object(object), func(func) {}
	
			inline return_type operator() (arg1_type a1) { 
				return (object->*func) (a1) ;
			} 
	
		private: 
			object_type *object; 
			func_t func;
		}; 

		
		template <typename arg1_type, class object_type>
		class slot1 <void, arg1_type, object_type> : public base_slot1 <void, arg1_type>{
		public: 
			typedef void (object_type::*func_t) (arg1_type a1) ;
			inline slot1 (object_type *object, func_t func) : object(object), func(func) {}
	
			inline void operator() (arg1_type a1) { 
				(object->*func) (a1); 
			} 
	
		private: 
			object_type *object; 
			func_t func; 
		};
		
		template <typename return_type, typename arg1_type>
		class base_signal1 {
		protected: 
			typedef base_slot1 <return_type, arg1_type> slot_type; 
			typedef std::deque<slot_type *> slots_type;
			slots_type slots;
		
		public: 
			inline virtual return_type emit (arg1_type a1) = 0;

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
			inline virtual ~base_signal1() {}
		};
		
		template <typename return_type, typename arg1_type, class validator_type = default_validator<return_type> >
		class signal1 : public base_signal1 <return_type, arg1_type> { 
		public: 
			typedef base_signal1 <return_type, arg1_type> parent_type; 
			
			inline virtual return_type emit (arg1_type a1) {
				return_type r; 
				for(typename parent_type::slots_type::iterator i = parent_type::slots.begin(); i != parent_type::slots.end(); ++i) { 
					r = (*i)->operator() (a1); 
				}
				return r; 
			} 
		};
			
		template <typename arg1_type, class validator_type >
		class signal1 <void, arg1_type, validator_type> : public base_signal1 <void, arg1_type> { 
		typedef base_signal1 <void, arg1_type> parent_type; 
		public: 
			inline void emit (arg1_type a1) {  
				for(typename parent_type::slots_type::iterator i = parent_type::slots.begin(); i != parent_type::slots.end(); ++i) { 
					(*i)->operator() (a1) ; 
				} 
			} 
		};
		
}

#endif

