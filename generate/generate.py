# https://github.com/posborne/cmsis-svd

# read access : register & field -> value
# read-only, write-only tags in svd
# registers that are 32 bits wide : direct value access
# peripherals that are 1 register: no struct?
# separate individual registers?

from cmsis_svd.parser import SVDParser

separator = "// %s\n" % ( 77 * "=" )
prefix = "hr"

def peripheral_prefix( peripheral ):
   if peripheral.prepend_to_name != None:
      return peripheral.prepend_to_name
   else:
      return ""   

def field_value( peripheral, register, field, value ):
   name = "%s%s_%s_%s" % ( peripheral_prefix( peripheral ), register.name.upper(), field.name.upper(), value.name.upper() )
   name = name.replace( "[0]", "" )
   s = ""
   if not ( "[" in name ):
      s += "      // %s\n" % value.description
      s += "      constexpr auto %s = %s::field_value_literal< 0x%08x, %d, %d >( %d );\n" % ( 
         name, prefix, peripheral.base_address + register.address_offset, field.bit_offset, field.bit_width, value.value ) 
   return s

def register_field( peripheral, register, field ):
   name = "%s%s_%s" % ( peripheral_prefix( peripheral ), register.name.upper(), field.name.upper() )
   name = name.replace( "[0]", "" )
   mask = "_Msk" if field.bit_width > 1 else ""
   s = ""
   if not ( "[" in name ):
      s += "   // %s\n" % field.description
      s += "   constexpr auto %s%s = %s::field_mask_literal< 0x%08x, %d, %d >();\n" % ( 
         name, mask, prefix, peripheral.base_address + register.address_offset, field.bit_offset, field.bit_width )    
      if field.enumerated_values != None : 
         for value in field.enumerated_values:
            s += field_value( peripheral, register, field, value )   
   return s

def register_fields( peripheral, register ):
   s = ""
   for field in register.fields:
      s += register_field( peripheral, register, field )
   return s

def register_type( peripheral, register ):
   address = peripheral.base_address + register.address_offset
   return "%s::hardware_register<0x%08x>" % ( prefix, address )

def register_definition( peripheral, register ):
   name = "%s_%s" % ( peripheral.name.upper(), register.name.upper() )
   address = peripheral.base_address + register.address_offset
   #name = name.replace( "[", "" ).replace( "]", "" ) 
   s = ""
   s += "// %s\n" % register.name
   s += "// %s\n" % register.description
   s += "#define %s ( * ( %s * ) 0x%08x )\n" % ( 
       name, register_type( peripheral, register ), address )
   return s

def peripheral_definition( peripheral, register ):
   name = "%s_%s" % ( peripheral.name.upper(), register.name.upper() )
   address = peripheral.base_address + register.address_offset
   #name = name.replace( "[", "" ).replace( "]", "" ) 
   s = ""
   s += "// %s\n" % register.name
   s += "// %s\n" % register.description
   s += "#define %s ( * ( %s * ) 0x%08x )\n" % ( 
       name, register_type( peripheral, register ), address )
   return s
   
def camel( s ):
   return s[ 0 ].upper() + s[ 1 : ].lower()   

def generate_peripheral( peripheral ):
   s = ""
   s += separator
   s += "//\n"
   s += "// %s\n" % peripheral.name
   s += "// base address = 0x%08x\n" % peripheral.base_address
   s += "// %s\n" % peripheral.description
   s += "//\n"
   s += separator
   
   s += "\n"
   s += "struct %s {\n" % camel( peripheral.name )
   delay = ""
   gather = "@"
   offset = 0
   sorted_peripherals = sorted( 
      peripheral.registers, key = lambda r : r.address_offset )
   for register in sorted_peripherals:

      # ignore alternate groups (they overlap, could be a union)
      if register.alternate_group != None:
         continue
         
      v = "   %s %s;\n" % \
         ( register_type( peripheral, register ), register.name.upper() )
         
         
      if offset != register.address_offset:
         s += delay
         delay = ""
         gather = "@"
         s += "   %s::reserved< 0x%X, %d > _reserved_at_0x%X;\n" % \
            ( prefix, offset, ( register.address_offset - offset ) // 4, offset )
      offset = register.address_offset + 4
         
      if register.name.startswith( gather ):
         delay = delay.replace( "[%d]" % ( n ), "[%d]" % ( n + 1 ) )
         n += 1      
         
      elif register.name.find( "[0]" ) > -1:
         s += delay
         delay = v.replace( "[0]", "[1]" )
         gather = register.name.split( "[0]" )[ 0 ]
         n = 1
         
         
      else:
         s += delay
         delay = ""
         gather = "@"
         s += v      
         
   s += delay      
   s += "};\n\n"
   
   s += "#define %s ( ( %s * ) 0x%08x )\n\n" % ( 
       peripheral.name.upper(), camel( peripheral.name ), peripheral.base_address )
       
   if not peripheral.name[ -1: ] in [ "1", "2", "3", "4", "5", "6", "7", "8", "9" ]:
    for register in sorted_peripherals:
      v = register_fields( peripheral, register )
      if v != "":
         s += "// %s\n" % register.name.upper()
         s += v
         s += "\n"
      
   return s

def generate_chip( manufacturer, chip ):
   device = SVDParser.for_packaged_svd( manufacturer, chip + ".svd" ).get_device()
   s = ""
   
   s += "#include \"hardware_registers.hpp\"\n"
   s += "namespace %s = hardware_registers;\n" % prefix
   s += "\n"
   
   s += separator
   s += "//\n"
   s += "// %s\n" % device.name
   s += "//\n"
   s += "// %s\n" % device.description
   s += "//\n"
   s += separator
   s += "\n"
   
   for peripheral in device.peripherals:
      s += generate_peripheral( peripheral )
      
   return s

# s = generate_chip( "STMicro", "STM32F401x" )
s = generate_chip( "Atmel", "ATSAM3X8E" )
open( "header.hpp", "w" ).write( s )

