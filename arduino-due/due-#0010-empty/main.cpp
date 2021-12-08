//#include <iostream>
#include <cstdint>
#include <new>

using register_value_type = uint32_t;

// ============================================================================
//
// A hardware register is 
//
// ============================================================================

template<
   int family,
   typename register_address_type,
   typename register_value_type
>
class hardware_register {
private:

   // =========================================================================
   // the memory-mapped register itself
   // =========================================================================
    
   volatile register_value_type the_hardware_register;
   
public:    

   // =========================================================================
   // mask of <number_of_bit> 1's, starting at <start_bit>
   // example: field_mask( 2, 3 ) == 0b0'111'00
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
   // an update for a subset of bits in the hardware register
   // =========================================================================
    
   template<
      int mask
   >

   struct update {
      const int value; 
      constexpr update( int value ): value( value ){}
      
      template< int right_mask >
         // the masks must no overlap
         requires( ( mask & right_mask ) == 0 )      
      update< mask | right_mask > operator+ ( 
         update< right_mask > right 
      ) const {
         return value | right.value;
      }
   };
    
   // =========================================================================
   // update operator
   // =========================================================================
    
   template<
      int mask
   >
   void operator << ( 
      update< mask > update
   ){
      the_hardware_register = ( the_hardware_register & mask ) | update.value;
   }      

   // =========================================================================
   // an update value created from a run-time value
   // =========================================================================
   
   template<
      int start_bit,
      int number_of_bits
   >
   struct value : update < 
      bit_mask( start_bit, number_of_bits ) 
   > {
      constexpr value( int value ): update< 
         bit_mask( start_bit, number_of_bits ) 
      >( value << start_bit ){}
   };

   // =========================================================================
   // an update value created from a compile-time value
   //
   // the value is checked to fit in the specified number_of_bits
   // =========================================================================
    
   template<
      int start_bit,
      int number_of_bits,
      int v
   >
      // the value v must fit in the specified number of bits
      requires( ( v & ~ bit_mask( 0, number_of_bits )) == 0 )
   struct literal : value < 
      start_bit, 
      number_of_bits
   > {
      constexpr literal(): value< 
         start_bit, 
         number_of_bits
      >( v ){}
   };

};

struct uart_control : public hardware_register< 0x40000000, uint32_t, uint32_t > {
public:    
   using super = hardware_register< 0x40000000, uint32_t, uint32_t >;
    //register_field< 0x40000000, 0, 1 > parity;
    
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
       
       using baudrate = super::value< 8, 5 >;
    // register_field< 0x40000000, 3, 5 > baudrate;
    
       // register_field_value< 0x40000000, 3, 5, 0b00 > handshake_none;    
       

};

struct uart {
    uart_control control;
};

#define uart1 ( ( uart * ) 0x40000000 )

int main(){

  uart1->control << 
      uart_control::parity_none +
      uart_control::handshake_software +
      uart_control::baudrate( 120 );
      
//prevents:
//   - multiple same field (but modify should be allowed - how?)     

   return 42;
}