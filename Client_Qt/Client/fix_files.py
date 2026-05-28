
import os
import re

def fix_file(file_path):
    with open(file_path, 'r', encoding='utf-8') as f:
        content = f.read()
    
    original = content
    
    # Fix HTML entities
    content = content.replace('&amp;', '&amp;')
    content = content.replace('&lt;', '&lt;')
    content = content.replace('&gt;', '&gt;')
    content = content.replace('&quot;', '"')
    content = content.replace('&#x27;', "'")
    
    # Also fix the ones that got double-encoded
    content = content.replace('&amp;amp;', '&amp;')
    content = content.replace('&amp;lt;', '&lt;')
    content = content.replace('&amp;gt;', '&gt;')
    
    if content != original:
        with open(file_path, 'w', encoding='utf-8') as f:
            f.write(content)
        print(f'Fixed: {file_path}')

def main():
    directory = r'D:\All Program\agant_example\Smart-Medica-Terminal\Client_Qt\Client'
    extensions = ['.cpp', '.h', '.pro']
    
    for root, dirs, files in os.walk(directory):
        for file in files:
            if any(file.endswith(ext) for ext in extensions):
                file_path = os.path.join(root, file)
                fix_file(file_path)

if __name__ == '__main__':
    main()
