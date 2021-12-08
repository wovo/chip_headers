// ============================================================================
//
// This set of classes provides a work-alike alternative to the 
// 'classic' C-style device header files typically provided by 
// micro-controller manufacturers. 
//
// The use case is that a number of errors are automatically detected
// (cause a compile-time error). The use must be in one of these forms:
//
// register &= ~ fields_mask
// register |= fields_value
// register = ( register & ~ fields_mask ) | fields_value
//
// where 
// - multiple fields_mask and fields_value can be or-combined,
//   but only when they belong to the same register
// - a fields_value can be created by left-shifting a literal value by
//   the offset of a field
// - fields_mask and fields_value must belong to the 
//   register that is accessed
// - the bits 'covered' by the mask and value must agree
//
// ============================================================================

#include <cstdint>

template<
   typename register_address_type  = uint32_t,
   typename register_value_type    = uint32_t
>   
struct  hardware_registers {

reg << value + value + field( v );
reg.field object, convert to typed value or bool or int 


// ============================================================================
// mask of <number_of_bit> 1's, starting at <start_bit>
// example: field_mask( 2, 3 ) == 0b0'111'00
// ============================================================================

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


// ============================================================================
// a field_mask
// ============================================================================

template<
   register_address_type  _class_register_address, 
   register_value_type    _used,
   register_value_type    _mask
>
struct field_mask {
   static constexpr int class_register_address  = _class_register_address;
   static constexpr register_value_type used     = _used;
   static constexpr register_value_type mask     = _mask;
   constexpr field_mask( int dummy = 0 ){}
};


// ============================================================================
// a field_mask literal
// specified by start_bit and number_of_bits
// ============================================================================

template< 
   register_address_type  _class_register_address, 
   int                    _start_bit, 
   int                    _number_of_bits 
>
struct field_mask_literal : field_mask <
   _class_register_address, 
   bit_mask( _start_bit, _number_of_bits ),
   bit_mask( _start_bit, _number_of_bits )
>{
   constexpr field_mask_literal( int dummy = 0 ){}
};


// ============================================================================
// the operator | (or) of two field_mask values
// ============================================================================

template< 
   register_address_type  _class_register_address, 
   register_value_type    _left_used,
   register_value_type    _left_mask,
   register_value_type    _right_used,
   register_value_type    _right_mask
>
field_mask< 
   _class_register_address, 
   _left_used | _right_used,
   _left_mask | _right_mask
> operator | (
   field_mask< _class_register_address, _left_used, _left_mask > left,
   field_mask< _class_register_address, _right_used, _right_mask > right
){
   return 0;
}   


// ============================================================================
// an inverted field_mask
// ============================================================================

template<
   register_address_type  _class_register_address, 
   register_value_type    _used,
   register_value_type    _mask
>
struct inverted_field_mask {
   static constexpr int class_register_address  = _class_register_address;
   static constexpr register_value_type used     = _used;
   static constexpr register_value_type mask     = _mask;
   constexpr inverted_field_mask( int dummy = 0 ){}
};


// ============================================================================
// the operator ~ (invert) of a field_mask
// ============================================================================

template< 
   register_address_type  _class_register_address, 
   register_value_type    _used,
   register_value_type    _mask
>
inverted_field_mask< 
   _class_register_address, 
   _used,
   _mask
> operator~ (
   field_mask< _class_register_address, _used, _mask > left
){
   return 0;
}   


// ============================================================================
// a masked register value 
// ============================================================================

template<
   register_address_type  _class_register_address, 
   register_value_type    _used,
   register_value_type    _mask
>
struct masked_register_value {
   static constexpr int class_register_address  = _class_register_address;
   static constexpr register_value_type used     = _used;
   static constexpr register_value_type mask     = _mask;
   constexpr masked_register_value( int dummy = 0 ){}
};


// ============================================================================
// a general field_value 
// ============================================================================

template<
   register_address_type  _class_register_address, 
   register_value_type    _used
>
struct field_value {
   static constexpr int class_register_address  = _class_register_address;
   static constexpr register_value_type used     = _used;
   
   const register_value_type value;
   
   constexpr field_value( register_value_type value ): value( value ){}
   
   constexpr operator uint32_t() const { return value; }
};


// ============================================================================
// a field_value literal
// specified by < start_bit, number_of_bits >( value )
// ============================================================================

template< 
   register_address_type   _class_register_address, 
   int                     _start_bit, 
   int                     _number_of_bits
>
struct field_value_literal : public field_value <
   _class_register_address, 
   bit_mask( _start_bit, _number_of_bits )
>{
   constexpr field_value_literal( register_value_type right ): 
      field_value <
         _class_register_address, 
         bit_mask( _start_bit, _number_of_bits )
      >( right << _start_bit ){}
};


// ============================================================================
// the operator | (or) of two field_value values
// ============================================================================

template< 
   register_address_type  _class_register_address, 
   register_value_type    _left_used,
   register_value_type    _right_used
>
field_value< 
   _class_register_address, 
   _left_used | _right_used
> operator | (
   field_value< _class_register_address, _left_used > left,
   field_value< _class_register_address, _right_used > right
){
   return ( left.value | right.value );
}   


// ============================================================================
// an updated_register_value
// specified by < start_bit, number_of_bits >( value )
// ============================================================================

template< 
   register_address_type   _class_register_address, 
   register_value_type     _and_mask,
   register_value_type     _or_used
>
struct updated_register_value {
   register_value_type or_value;
   
   constexpr updated_register_value( register_value_type value ): 
   or_value( value ){}
};


// ============================================================================
// the operator | (or) of a masked_register_value and a field_value
// ============================================================================

template< 
   register_address_type  _class_register_address, 
   register_value_type    _and_mask,
   register_value_type    _or_used
>
updated_register_value< 
   _class_register_address, 
   _and_mask,
   _or_used
> operator | (
   masked_register_value< _class_register_address, _and_mask, _and_mask > left,
   field_value< _class_register_address, _or_used > right
){
   return right.value;
}   





// ============================================================================
// a hardware register
// register &= operator
// register |= operator
// register = operator
// ============================================================================

         
template<
   register_address_type _class_register_address       
>
struct hardware_register {
   
   volatile register_value_type the_register;   
   
   // =========================================================================
   // operator & ( field_mask )
   // =========================================================================

   template<
      register_value_type _used,
      register_value_type _mask
   >
   register_value_type operator & (
      field_mask< _class_register_address, _used, _mask > rhs
   ) const {
      return the_register & _mask;
   }         
   
   // =========================================================================
   // operator & ( inverted_field_mask )
   // =========================================================================
   
   template< 
      register_value_type     _used,
      register_value_type     _mask
   >
   masked_register_value< 
      _class_register_address, 
      _used,
      _mask
   > operator & (
      inverted_field_mask< _class_register_address, _used, _mask > right
   ){
      return 0;
   }      
      
   // =========================================================================
   // operator = ( register_value_type )
   // =========================================================================

   void operator = (
      register_value_type rhs
   ){
      the_register = rhs;
   }      
   
   // =========================================================================
   // operator &= ( inverted_field_mask )
   // =========================================================================

   template<
      register_value_type _used,
      register_value_type _mask
   >
   void operator &= (
      inverted_field_mask< _class_register_address, _used, _mask > rhs
   ){
      the_register = the_register & ~ _mask;
   }      
   
   // =========================================================================
   // operator |= ( field_value )
   // =========================================================================

   template<
      register_value_type _used
   >
   void operator |= (
      field_value< _class_register_address, _used > rhs
   ){
      the_register = the_register | rhs.value;
   }      

   // =========================================================================
   // operator = ( field_value )
   // =========================================================================

   template<
      register_value_type _used
   >
   void operator = (
      field_value< _class_register_address, _used > rhs
   ){
      the_register = rhs.value;
   }      
   
   // =========================================================================
   // operator = ( field_mask )
   // =========================================================================

   template<
      register_value_type _used,
      register_value_type _mask
   >
   void operator = (
      field_mask< _class_register_address, _used, _mask > rhs
   ){
      the_register = _mask;
   }      
   
   // =========================================================================
   // operator = ( updated_register_value )
   // =========================================================================

   template<
      register_value_type _and_mask,
      register_value_type _or_used
   >
   void operator = (
      updated_register_value< _class_register_address, _and_mask, _or_used > rhs
   ){
      the_register = ( the_register & ~ _and_mask ) | rhs.or_value;
   }      
   
};


// ============================================================================
//
// used for filling reserved locations within a device
//
// ============================================================================

template<
   register_address_type  _class_register_address,       
   int                    _number_of_words      
>
struct reserved {
private:   
   volatile register_value_type words[ _number_of_words ]; 
};

   
// ============================================================================
// end of namespace hardware_registers
// ============================================================================

}; 


