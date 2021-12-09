// ===========================================================================
//
// The hardware_register class template provides a mechanism for abstracting
// a memory-mapped peripheral hardware register type, 
// like a UART configuration register.
//
// The intended use is that an actual register class inherits from
// this class, and providing declarations for the registers fields and
// possibly named values for those fields, using the provided abstractions.
//
// Such register definitions can be created by hand from the information 
// in the data-sheet, but generating it, for instance from cortex svd files,
// is a less error-prone process.
//
// An instance of such a concrete register class can be placed at an
// absolute memory address with the standard #define trick.
// Using placement new would be a better (scoped!) alternative, but it
// creates global constructor calls that I'd rather avoid.
//
// An example of a register definition generated for the uart 
// <>
// 
// ===========================================================================

#include <cstdint>

template<

      // =====================================================================
      //
      // the type that specified an address, 
      // like uint32_t for a cpu with a 32 bit address range
      //
      // =====================================================================
      
   typename register_address_type,
   
      // =====================================================================
      //
      // the type that specifies a hardware register,
      // for most 32-bit cpu's this will be uint32_t
      //
      // =====================================================================
      
   typename register_value_type,
   
      // =====================================================================
      //
      // a value that uniquely identifies the register
      // When the chip has only one register of this type 
      // it can be its address.
      // When the chip has multiple registers of this type, it
      // can be the address of the first of these registers.
      // The value is only used to uniquely identify the type of 
      // the register.
      //
      // =====================================================================

   register_address_type family
   
>  
class hardware_register {
private:

   // ========================================================================
   // the memory-mapped register itself
   // ========================================================================
    
   volatile register_value_type the_hardware_register;
   
public:    

   // ========================================================================
   //
   // This function returns a mask of <number_of_bit> 1's, 
   // starting at <start_bit>.
   //
   // example: bit_mask( 2, 3 ) == 0b0'111'00
   //
   // All bit_masks used in this class are created at compile time.
   //
   // ========================================================================
    
   static constexpr register_value_type bit_mask( 
      int start_bit, 
      int number_of_bits  
   ){
      if( number_of_bits == 0 ){
         return 0;
      } else if( start_bit == 0 ){
         return 0b01 | ( ( bit_mask( 0, number_of_bits - 1 ) << 1 ) );
      } else {
         return bit_mask( 0, number_of_bits ) << start_bit;
      }   
   }   
   
   
   // ========================================================================
   //
   // An update value defines an update for a subset of bits 
   // in the hardware register (in data-sheet this is often called a field).
   // 
   // The mask has 1 bits (only) fore the bits that are to be updated.
   // The value can only have 1 bits in places where the mask has a 1 bit
   // (this is enforced by all constructors and operations).
   //
   // ========================================================================
    
   template<
      register_value_type mask
   >
   struct update {
      const register_value_type value; 
      
      constexpr update( register_value_type value ): value( value ){}
      
      
      // =====================================================================
      //
      // The + operator combines two update values.
      // Both the mask and the value are combined by or-ing.
      //
      // The masks are not allowed to overlap, because this would
      // (potentially) require two different values for the overlapping bits.
      //
      // =====================================================================
      
      template< register_value_type right_mask >
         requires( 
            // the masks must no overlap
            ( mask & right_mask ) == 0 
         )      
         __attribute__((always_inline)) 
         [[ nodiscard ]]
         constexpr 
      update< mask | right_mask > operator+ ( 
         update< right_mask > right 
      ) const {
         return value | right.value;
      }
      
      
      // =====================================================================
      //
      // The << operator returns a copy of the left value, with all bits
      // specified in the right value updated to those values.
      //
      // =====================================================================
      
      template< register_value_type right_mask >
         requires( 
            // the masks must no overlap
            ( mask & right_mask ) == 0 
         )      
         __attribute__((always_inline)) 
         [[ nodiscard ]]
         constexpr 
      update< mask | right_mask > operator<< ( 
         update< right_mask > right 
      ) const {
         return ( value & ~right.mask ) | right.value;
      }
      
   };
    
    
   // ========================================================================
   //
   // The <<= is used as register update operator: 
   // it updates the bits specified in the update value to the value
   // that is also specified in the update value.
   //
   // When the update specifies the value for all bits, it
   // is written to the register without first reading is value.
   //
   // ========================================================================
    
   template<
      register_value_type mask
   >
      __attribute__((always_inline)) 
   void operator <<= ( 
      update< mask > update
   ){
      if constexpr( 0 == ~ mask ){
         the_hardware_register = update.value;
      } else {
         the_hardware_register = 
            ( the_hardware_register & ~ mask ) | update.value;
      }         
   }      


   // ========================================================================
   //
   // A fields is a number of adjacent bits within the register,
   // specified by the start_bit and number_of_bits.
   //
   // ========================================================================
      
   template<
      int _start_bit,
      int _number_of_bits
   >
   struct field {
       
      static constexpr auto start_bit = _start_bit;      
      static constexpr auto number_of_bits = _number_of_bits;      
       
      // =====================================================================
      //
      // This is update value for the field, created at run-time.
      //
      // The all bits outside the specified number_of_bits
      // are cleared (forced to 0).
      //
      // Use this (only) when the value is not known at compile-time:
      // the literal<> template has less overhead and checks
      // (at compile-time) that the value is within range.
      //
      // =====================================================================
      
      struct value : update< 
         bit_mask( start_bit, number_of_bits ) 
      >{      
   
            __attribute__((always_inline))
            // [[ nodiscard ]] - gives false positives?
            constexpr 
         value ( 
            register_value_type value 
         ): update<
            bit_mask( start_bit, number_of_bits ) 
         > (
            ( bit_mask( 0, number_of_bits ) & value ) << start_bit 
         ){}
      
      };
      
      // =====================================================================
      //
      // The literal class template is an update value for the field, 
      // created at compile-time.
      //
      // The value is (at compile-time) checked to fit in 
      // the specified number_of_bits.
      //
      // =====================================================================
    
      template<
         register_value_type v
      >
         requires( 
            // the value v must fit in the specified number of bits
            ( v & ~ bit_mask( 0, number_of_bits ) ) == 0 
         )
      struct literal : update< 
         bit_mask( start_bit, number_of_bits ) 
      >{      
      
            __attribute__((always_inline))
            // [[ nodiscard ]] - gives false positives??
            constexpr
         literal() : update < 
            bit_mask( start_bit, number_of_bits ) 
         >(
             v << start_bit 
         ){}       
      };  
         
   }; // struct template field
      
   template< typename T >
   constexpr register_value_type read() const {
      return 
         ( the_hardware_register >> T::start_bit )
         & bit_mask( 0, T::number_of_bits );
   }
  
}; // class template hardware_register


// ===========================================================================
//
// Check, for the typical cases of an 8, 16 and 32 bits CPU, that the size 
// of a hardware_register instantiation is just the register itself.
//
// ===========================================================================

static_assert( sizeof( hardware_register< uint8_t,  uint8_t,  0 > ) == 1 );
static_assert( sizeof( hardware_register< uint16_t, uint8_t,  0 > ) == 1 );
static_assert( sizeof( hardware_register< uint16_t, uint16_t, 0 > ) == 2 );
static_assert( sizeof( hardware_register< uint32_t, uint32_t, 0 > ) == 4 );


struct uart_control : public hardware_register< uint32_t, uint32_t, 0x40000000 > {
private:
   using super = hardware_register< uint32_t, uint32_t, 0x40000000 >;
    
public:    
   
   using all = super::field< 0, 31 >;
       
   using parity = super::field< 0, 2 >;
   auto parity_value(){ return super::read< parity >(); }
      static constexpr parity::literal<  0 > parity_none = {};
      static constexpr parity::literal<  1 > parity_even = {};
      static constexpr parity::literal<  2 > parity_odd = {};
       
   using handshake = super::field< 3, 2 >;
      static constexpr handshake::literal< 0b00 > handshake_none = {};
      static constexpr auto handshake_software = handshake::literal< 0b01 >();
      static constexpr auto handshake_hardware = handshake::literal< 0b10 >();
       
   using baudrate = super::field< 8, 9 >;
      static constexpr auto baudrate_120 = baudrate::literal< 120 >();

   // get the raw register value
   // set the raw register value - value( NN )   
       
   // get the typed field value
   // get the raw field value
   // set the raw field value - field( NN )
};


struct uart {
    uart_control control;
};


#define uart1 ( ( uart * ) 0x40000000 )
#define uart2 ( * ( volatile uint32_t * ) 0x40000000 )


//0b11111111_000011011  
//0b01111000_000001000

   static_assert( sizeof( uart_control ) == 4 );
   static_assert( sizeof( uart ) == 4 );

int main(){
    
   return sizeof( hardware_register< uint32_t, uint32_t, 0x40000000 > );
   return sizeof( uart_control );
   uart1->control <<= uart_control::all::value( 1234 );   
   
if(0){
   uart2 = ( uart2 & ~ 0b11111111'000011011UL ) | 0b01111000'000001000UL;
}   

if(0){
  uart1->control <<=
      uart_control::parity_none +
      uart_control::handshake_software +
      uart_control::baudrate::value( 120 );
}      

auto x = uart1->control.parity_value();
// get_parity
uart1->control <<= uart_control::baudrate::value( x + 1 );
      
if(0){
  constexpr auto v = 
      uart_control::parity_none +
      uart_control::handshake_software +
      uart_control::baudrate_120;
      
     uart1->control <<= v;    
       
}     

   return 42;
}