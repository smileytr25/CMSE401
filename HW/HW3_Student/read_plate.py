
from PIL import Image
import numpy as np

img = Image.open('plate.png').convert('L')
binary = (np.array(img) > 128).astype(int)
print(binary)

