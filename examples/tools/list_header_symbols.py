#!../.venv/bin/python
"""
Script to list all symbols declared in a header file.
Uses clang to parse the header and extract symbol names.
"""

import clang.cindex
import sys
import os

# --- (Optional) Manual configuration if libclang is not found automatically ---
# clang.cindex.Config.set_library_file('/usr/lib/llvm-14/lib/libclang.so') 

def extract_symbols_from_header(header_filename):
    """
    Parses a header file and extracts all declared symbols.
    Returns a set of symbol names.
    """
    index = clang.cindex.Index.create()
    symbols = set()
    
    print(f"Parsing {header_filename}...", file=sys.stderr)
    
    # Parse the header file
    tu = index.parse(header_filename, args=['-std=c99'])
    
    if not tu:
        print(f"Error: Unable to parse translation unit for {header_filename}", file=sys.stderr)
        return symbols

    def process_cursor(cursor, depth=0):
        """Recursively process cursors to find all declarations."""
        
        # Only consider declarations from the target header file
        if cursor.location.file and os.path.abspath(cursor.location.file.name) != os.path.abspath(header_filename):
            return
        
        # Skip anonymous declarations
        if not cursor.spelling:
            # Still recurse into children for anonymous structs/unions
            for child in cursor.get_children():
                process_cursor(child, depth + 1)
            return
        
        # Function declarations
        if cursor.kind == clang.cindex.CursorKind.FUNCTION_DECL:
            # Skip static inline functions if desired, or include them
            # if cursor.linkage != clang.cindex.LinkageKind.INTERNAL:
            symbols.add(cursor.spelling)
        
        # Variable declarations (global variables, extern variables)
        elif cursor.kind == clang.cindex.CursorKind.VAR_DECL:
            symbols.add(cursor.spelling)
        
        # Typedef declarations
        elif cursor.kind == clang.cindex.CursorKind.TYPEDEF_DECL:
            symbols.add(cursor.spelling)
        
        # Struct declarations
        elif cursor.kind == clang.cindex.CursorKind.STRUCT_DECL:
            symbols.add(cursor.spelling)
        
        # Union declarations
        elif cursor.kind == clang.cindex.CursorKind.UNION_DECL:
            symbols.add(cursor.spelling)
        
        # Enum declarations
        elif cursor.kind == clang.cindex.CursorKind.ENUM_DECL:
            symbols.add(cursor.spelling)
        
        # Enum constant (enumerator)
        elif cursor.kind == clang.cindex.CursorKind.ENUM_CONSTANT_DECL:
            symbols.add(cursor.spelling)
        
        # Macro definitions
        elif cursor.kind == clang.cindex.CursorKind.MACRO_DEFINITION:
            symbols.add(cursor.spelling)
        
        # Field declarations (struct/union members)
        elif cursor.kind == clang.cindex.CursorKind.FIELD_DECL:
            # Optionally add field names
            # symbols.add(cursor.spelling)
            pass
        
        # Recurse into children to catch nested declarations
        for child in cursor.get_children():
            process_cursor(child, depth + 1)
    
    # Start processing from the translation unit's root cursor
    process_cursor(tu.cursor)
    
    return symbols


if __name__ == "__main__":
    if len(sys.argv) != 2:
        print(f"Usage: {sys.argv[0]} <header_file.h>", file=sys.stderr)
        sys.exit(1)
    
    header_file = sys.argv[1]
    
    # Check if file exists
    if not os.path.exists(header_file):
        print(f"Error: File '{header_file}' not found", file=sys.stderr)
        sys.exit(1)
    
    # Extract symbols
    symbols = extract_symbols_from_header(header_file)
    
    # Output sorted list of symbols
    if symbols:
        print(",".join(sorted(symbols)))

    else:
        print("0", file=sys.stderr) #code for reset filter if no symbols found => all kernel symbols will be included
        sys.exit(1)
