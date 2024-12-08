import os

def print_directory_tree(startpath, max_depth=2, current_depth=0):
    # Get the base name of the starting path
    base_name = os.path.basename(startpath)
    
    # Define directories to exclude
    excluded_dirs = {'.git', 'build', '.vscode'}
    
    # Only print the directory name if at the current depth
    if current_depth <= max_depth:
        print('    ' * current_depth + f'├── {base_name}/')
    
        # List entries in the directory
        for entry in os.listdir(startpath):
            entry_path = os.path.join(startpath, entry)

            # Skip excluded directories
            if entry in excluded_dirs and os.path.isdir(entry_path):
                continue
            
            # Print directories and files as needed
            if os.path.isdir(entry_path):
                print_directory_tree(entry_path, max_depth, current_depth + 1)
            elif entry == "CMakeLists.txt" or entry.endswith(".c"):
                print('    ' * (current_depth + 1) + f'└── {entry}')

# Usage
if __name__ == '__main__':
    # Expand the home directory
    project_directory = os.path.dirname(os.path.abspath(__file__))
    print_directory_tree(project_directory)
