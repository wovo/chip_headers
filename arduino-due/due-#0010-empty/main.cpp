
// ============================================================================
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
// ============================================================================

template<

      // the type that specified an address, 
      // like uint32_t for a cpu with a 32 bit address range
   typename register_address_type,
   
      // the type that specifies a hardware register,
      // for most 32-bit cpu's this will be uint32_t
   typename register_value_type,
   
      // a value that uniquely identifies the register
      // When the chip has only one register of this type 
      // it can be its address.
      // When the chip has multiple registers of this type, it
      // can be the address of the first of these registers.
      // The value is only used to uniquely identify the type of 
      // the register.
   register_address_type family
   
>  
class hardware_register {
private:

   // =========================================================================
   // the memory-mapped register itself
   // =========================================================================
    
   volatile register_value_type the_hardware_register;
   
public:    

   // =========================================================================
   // This function returns a mask of <number_of_bit> 1's, 
   // starting at <start_bit>.
   //
   // example: bit_mask( 2, 3 ) == 0b0'111'00
   //
   // All bit_masks used in this class are created at compile time.
   // =========================================================================
    
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
   
   // =========================================================================
   // An update value defines an update for a subset of bits 
   // in the hardware register. 
   // The mask has 1 bits (only) fore the bits that are to be updated.
   // The value can only have 1 bits in places where the mask has a 1 bit.
   // =========================================================================
    
   template<
      register_value_type mask
   >
   struct update {
      const register_value_type value; 
      
      constexpr update( register_value_type value ): value( value ){}
      
      // ======================================================================
      // The + operator combines two update values.
      // Both the mask and the value are combined by or-ing.
      //
      // The masks are not allowed to overlap, because this would
      // (potentially) require two different values for the overlapping bits.
      // ======================================================================
      template< register_value_type right_mask >
         // the masks must no overlap
         requires( ( mask & right_mask ) == 0 )      
      update< mask | right_mask > 
      constexpr __attribute__((always_inline)) 
      operator+ ( 
         update< right_mask > right 
      ) const {
         return value | right.value;
      }
   };
    
   // =========================================================================
   // The << is used as update operator: 
   // it updates the bits specified in the update value to the value
   // that is also specified in the update value.
   //
   // When the update specifies the value for all bits, it
   // is written to the register without first reading is value.
   // =========================================================================
    
   template<
      register_value_type mask
   >
   void __attribute__((always_inline)) 
   operator << ( 
      update< mask > update
   ){
      if constexpr( 0 == ~ mask ){
         the_hardware_register = update.value;
      } else {
         the_hardware_register = 
            ( the_hardware_register & ~ mask ) | update.value;
      }         
   }      

   // =========================================================================
   // This is an update value created at run-time.
   //
   // The all bits outside the specified number_of_bits
   // are cleared (forced to 0).
   //
   // Use this (only) when the value is not known at compile-time:
   // the literal<> template has less overhead and checks
   // (at compile-time) that the value is within range.
   // =========================================================================
   
   template<
      int start_bit,
      int number_of_bits
   >
   struct value : update < 
      bit_mask( start_bit, number_of_bits ) 
   > {
      constexpr __attribute__((always_inline))
      value( register_value_type value ): update< 
         bit_mask( start_bit, number_of_bits ) 
      >( ( bit_mask( 0, number_of_bits ) & value ) << start_bit ){}
   };

   // =========================================================================
   // This is an update value created at compile-time.
   //
   // The value is (at compile-time) checked to fit in 
   // the specified number_of_bits.
   // =========================================================================
    
   template<
      int start_bit,
      int number_of_bits,
      register_value_type v
   >
      // the value v must fit in the specified number of bits
      requires( ( v & ~ bit_mask( 0, number_of_bits )) == 0 )
   struct literal : value < 
      start_bit, 
      number_of_bits
   > {
      constexpr __attribute__((always_inline))
      literal(): value< 
         start_bit, 
         number_of_bits
      >( v ){}
   };

}; // class template hardware_register

#include <cstdint>

struct uart_control : public hardware_register< uint32_t, uint32_t, 0x40000000 > {
public:    
   using super = hardware_register< uint32_t, uint32_t, 0x40000000 >;
    //register_field< 0x40000000, 0, 1 > parity;
    using value = super::value< 0, 31 >;
       
    
       super::value< 0, 2 > parity;
       static constexpr super::literal<  0,  2,   0 > parity_none = {};
       static constexpr super::literal<  0,  2,   1 > parity_even = {};
       static constexpr super::literal<  0,  2,   2 > parity_odd = {};
       //register_field_value< 0x40000000, 0, 1, 0b1 > parity_odd;
       
    //register_field< 0x40000000, 1, 2 > handshake;
    
       // static handshake = register_field_value< 0x40000000, 1, 2 >;
       static constexpr auto handshake_none = super::literal< 3, 2, 0b00 >();
       static constexpr auto handshake_software = super::literal< 3, 2, 0b01 >();
       static constexpr auto handshake_hardware = super::literal< 3, 2, 0b10 >();
       
       using baudrate = super::value< 8, 9 >;
       static constexpr auto baudrate_120 = super::literal< 8, 8, 120 >();
    // register_field< 0x40000000, 3, 5 > baudrate;
    
       // register_field_value< 0x40000000, 3, 5, 0b00 > handshake_none;    
       

};


struct uart {
    uart_control control;
};

/*
#define uart1 ( ( uart * ) 0x40000000 )
#define uart2 ( * ( volatile uint32_t * ) 0x40000000 )
*/

//0b11111111_000011011  
//0b01111000_000001000


int main(){
   
   uart1->control << uart_control::value( 1234 );   
   
if(0){
   uart2 = ( uart2 & ~ 0b11111111'000011011UL ) | 0b01111000'000001000UL;
}   

if(0){
  uart1->control <<
      uart_control::parity_none +
      uart_control::handshake_software +
      uart_control::baudrate_120;
}      
      
if(0){
  constexpr auto v =  
      uart_control::parity_none +
      uart_control::handshake_software +
      uart_control::baudrate_120;
      
     uart1->control << v;    
}     


    
//prevents:
//   - multiple same field (but modify should be allowed - how?)     

   return 42;
}