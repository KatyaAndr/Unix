if __name__ == '__main__':
    file_size = 4 * 1024 * 1024 + 1

    bytes_to_write = bytearray([0] * file_size)
    bytes_to_write[0] = 1
    bytes_to_write[10000] = 1
    bytes_to_write[-1] = 1

    with open('run/A', mode='wb') as file:
        file.write(bytes_to_write)
        