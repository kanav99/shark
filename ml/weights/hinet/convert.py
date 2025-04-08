import torch
import struct

state_dict = torch.load("weights.pt", map_location=torch.device('cpu'))

with open("weights.dat", 'wb') as f:
  for k in state_dict:
    dims = len(state_dict[k].size())
    if dims == 4:
        # pytorch uses CO x CI x FH x FW
        # we use CO x FH x FW x CI
        # permute according to our format
        filter_mat = state_dict[k].permute(0, 2, 3, 1)
        filter_mat = filter_mat.numpy()
        # flatten
        packed = struct.pack(f'{filter_mat.flatten().size}f', *filter_mat.flatten())
        f.write(packed)
    elif dims == 2:
        # pytorch uses CO x CI
        # we use CI x CO
        # permute according to our format
        mat = state_dict[k].permute(1, 0)
        mat = mat.numpy()
        # flatten
        packed = struct.pack(f'{mat.flatten().size}f', *mat.flatten())
        f.write(packed)
    elif dims == 1:
        w = state_dict[k].numpy()
        packed = struct.pack(f'{w.flatten().size}f', *w.flatten())
        f.write(packed)
