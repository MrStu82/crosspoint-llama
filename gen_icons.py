import os

def hexify(bits):
    res = []
    for i in range(0, len(bits), 8):
        byte_str = bits[i:i+8]
        val = int(byte_str, 2)
        res.append(f"0x{val:02X}")
    return res

def make_icon(size):
    # size is either 32 or 24
    lines = []
    lines.append('#pragma once\n#include <cstdint>\n\n')
    lines.append(f'// size: {size}x{size}\n')
    lines.append(f'static const uint8_t Game{"24" if size==24 else ""}Icon[] = {{\n')
    
    bytes_arr = []
    for r in range(size):
        row_bits = ""
        for c in range(size):
            # very simple gamepad:
            # top/bottom padding
            if r < size*0.3 or r > size*0.7:
                row_bits += "1"
            else:
                # L/R bounds
                if c < size*0.1 or c > size*0.9:
                    row_bits += "1"
                else:
                    # inner logic
                    if size == 32:
                        # cross
                        is_cross = (r > 12 and r < 20 and c > 5 and c < 9) or (r > 14 and r < 18 and c > 3 and c < 11)
                        # buttons
                        is_btn = (r > 14 and r < 18 and c > 21 and c < 25)
                        if is_cross or is_btn:
                            row_bits += "1" # white
                        else:
                            row_bits += "0" # black
                    else:
                        is_cross = (r > 9 and r < 15 and c > 4 and c < 7) or (r > 11 and r < 13 and c > 2 and c < 9)
                        is_btn = (r > 11 and r < 13 and c > 16 and c < 19)
                        if is_cross or is_btn:
                            row_bits += "1"
                        else:
                            row_bits += "0"
        bytes_arr.extend(hexify(row_bits))
        
    lines.append('    ' + ', '.join(bytes_arr))
    lines.append('};\n')
    return ''.join(lines)

with open('src/components/icons/game.h', 'w') as f:
    f.write(make_icon(32))
with open('src/components/icons/game24.h', 'w') as f:
    f.write(make_icon(24))
