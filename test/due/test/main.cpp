// ========================================================

constexpr register_value_type bit_mask( 
   int start_bit, 
   int number_of_bits 
){
   if( number_of_bits == 0 ){
      return 0;
   } else if( start_bit == 0 ){
      return 0b01 | ( bit_mask( 0, number_of_bits - 1 ) << 1 );
   } else {
      return bit_mask( 0, number_of_bits ) << start_bit;
   }   
}

// ========================================================

template<
   int address,
   int mask
> 
class register_field_value {
private:
   int value;
public:
   register_field_value( int v ): v( v ){}
};

// ========================================================

template<
   int address,
   int left_mask,
   int right_mask
> 
register_field_value< 
   address, 
   left_mask | right_mask 
>
operator + (
   register_field_value< address, left_mask > left,
   register_field_value< address, right_mask > right
){
    return left.value | right.value;
}

// ========================================================

template<
   int address,
   int start_bit,
   int number_of_bits
> 
register_field_value<
   address,
   bit_mask( start_bit, number_of_bits )
>
register_field_literal( 
   int value 
){
   return value;
}   

};

class registet_control {
private:
    hardware_register;
public:    
    //register_field< 0x40000000, 0, 1 > parity;
    
       constexpr auto parity_even = register_field_literal< 0x40000000, 0, 1 >( 0b0 );
       constexpr auto parity_odd  = register_field_literal< 0x40000000, 0, 1 >( 0b2 );
       register_field_value< 0x40000000, 0, 1, 0b1 > parity_odd;
       
    //register_field< 0x40000000, 1, 2 > handshake;
    
       // static handshake = register_field_value< 0x40000000, 1, 2 >;
       constexpr auto handshake_none = register_field_value< 0x40000000, 1, 2, 0b00 > 
       constexpr auto handshake_software = register_field_value< 0x40000000, 1, 2, 0b01 > 
       constexpr auto handshake_hardware = register_field_value< 0x40000000, 1, 2, 0b10 > 
       
    // register_field< 0x40000000, 3, 5 > baudrate;
    
       // register_field_value< 0x40000000, 3, 5, 0b00 > handshake_none;    
};

struct uart_struct {
   uart_control control;
};

uart_struct ss[1];

//auto uart1 = new( 0x40000000 ) uart_struct;
auto uart1 = new( s ) uart_struct;

int main(){
   uart1->control << 
      uart_control::parity_odd  +
      uart_control::handshake_off;
      // uart_control::baudrate( 1200 );
      
//prevents:
//   - field from different register
//   - multiple same field (but modify should be allowed - how?)     
}