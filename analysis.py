def count_and_size(file_path):
    w_count = r_count = 0
    w_size = r_size = 0

    with open(file_path, 'r') as file:
        for line in file:
            parts = line.strip().split(',')
            # 确保每行都有至少4个元素
            if len(parts) >= 4 and parts[3].lower() in ('w', 'r'):
                # 通过.lower()来忽略大小写
                if parts[3].lower() == 'w':
                    w_count += 1
                    w_size += int(parts[2])
                elif parts[3].lower() == 'r':
                    r_count += 1
                    r_size += int(parts[2])

    # 转换字节为GB
    w_size_gb = w_size / (2**30)
    r_size_gb = r_size / (2**30)

    return w_count, r_count, w_size_gb, r_size_gb

file_path = r'C:\Users\86159\Desktop\trash\Trace\Financial1.spc'
w_count, r_count, w_size_gb, r_size_gb = count_and_size(file_path)

print(f"'w' or 'W' count: {w_count}, size: {w_size_gb:.2f} GB")
print(f"'r' or 'R' count: {r_count}, size: {r_size_gb:.2f} GB")
