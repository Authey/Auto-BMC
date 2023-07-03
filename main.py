import re
import os
import subprocess
from tkinter import *
from tkinter.filedialog import askdirectory


def select_path(path):
    path_ = askdirectory()
    path.set(path_)


def get_func():
    if not tar_path.get():
        tar_path.set('.')
    files = os.listdir(tar_path.get())
    funcs = {}
    for file in files:
        if not os.path.isdir('{}/{}'.format(tar_path.get(), file)) and re.search('\.c$', file):
            funcs[file] = []
            with open('{}/{}'.format(tar_path.get(), file)) as f:
                iter_f = iter(f)
                for line in iter_f:
                    func_obj = re.search('^(\w+ )?\w+ (\* )?(\w+ )?\w+( )?\(', line)
                    if func_obj:
                        funcs[file].append(re.search('\w+( )?\(', func_obj.group()).group()[:-1])
    return funcs


def verify_func():
    func_dic = get_func()
    if not out_path.get():
        out_path.set('.')
    with open('{}/{}_output.txt'.format(out_path.get(), bmc.get()), 'a') as f_out:
        with open('{}/{}_error.txt'.format(out_path.get(), bmc.get()), 'a') as f_err:
            for file in func_dic:
                for func in func_dic[file]:
                    subprocess.Popen('echo \"\n======= {} verification for function {} in file {} =======\" >> {}/{}_output.txt'.format(bmc.get(), func, file, out_path.get(), bmc.get()), shell=True)
                    subprocess.Popen('echo \"\n======= {} verification for function {} in file {} =======\" >> {}/{}_error.txt'.format(bmc.get(), func, file, out_path.get(), bmc.get()), shell=True)
                    if inc_path.get():
                        p = subprocess.Popen('{} -I {} {}/{} --unwind 1000 {} --function {}'.format(bmc.get(), inc_path.get(), tar_path.get(), file, options.get(), func), shell=True, stdout=f_out, stderr=f_err)
                    else:
                        p = subprocess.Popen('{} {}/{} --unwind 1000 {} --function {}'.format(bmc.get(), tar_path.get(), file, options.get(), func), shell=True, stdout=f_out, stderr=f_err)
                    text_box.insert(END, '{} verification done for function {}.\n'.format(bmc.get(), func))
                    text_box.see(END)
                    text_box.update()
                    while p.poll() is None:
                        p.wait()


'''
Possible Improvements:
    - Empty function filter
    - Missing file autocompletion
    - Definition issue autodebug (add or fix)
    - Program level support
'''
if __name__ == '__main__':
    # parser = argparse.ArgumentParser()
    # parser.add_argument('-v', '--verbose')
    # parser.add_argument('-t', '--target', nargs=None, default='.')
    # parser.add_argument('-o', '--output', nargs=None, default='.')
    # parser.add_argument('-i', '--include', nargs=None, default=None)
    # parser.add_argument('-c', '--bmc', nargs=None, default='cbmc')
    # parser.add_argument('--option', nargs='+', default=None)
    # args = parser.parse_args()
    # if args.bmc != 'cbmc' and args.bmc != 'esbmc':
    #     print('{} is not supported, please use cbmc or esbmc.'.format(args.bmc))
    #     exit()
    # functions = get_func(args.target)
    # verify_func(functions, args)
    root = Tk()
    root.title('Verification Tool')
    bmc = StringVar()
    tar_path = StringVar()
    inc_path = StringVar()
    out_path = StringVar()
    options = StringVar()
    Label(root, text='Model Checker:').grid(row=0, column=0, padx=(10, 5))
    Radiobutton(root, text='CBMC', variable=bmc, value='cbmc').grid(row=0, column=1)
    Radiobutton(root, text='ESBMC', variable=bmc, value='esbmc').grid(row=0, column=2)
    bmc.set('cbmc')
    Label(root, text='Target Directory:').grid(row=1, column=0, padx=(10, 5))
    Entry(root, textvariable=tar_path).grid(row=1, column=1, columnspan=2, ipadx=60)
    Button(root, text='Select', command=lambda: select_path(tar_path)).grid(row=1, column=3, padx=(5, 10))
    Label(root, text='Include Directory:').grid(row=2, column=0, padx=(10, 5))
    Entry(root, textvariable=inc_path).grid(row=2, column=1, columnspan=2, ipadx=60)
    Button(root, text='Select', command=lambda: select_path(inc_path)).grid(row=2, column=3, padx=(5, 10))
    Label(root, text='Output Directory:').grid(row=3, column=0, padx=(10, 5))
    Entry(root, textvariable=out_path).grid(row=3, column=1, columnspan=2, ipadx=60)
    Button(root, text='Select', command=lambda: select_path(out_path)).grid(row=3, column=3, padx=(5, 10))
    Label(root, text='BMC Options:').grid(row=4, column=0, padx=(10, 5))
    Entry(root, textvariable=options).grid(row=4, column=1, columnspan=2, ipadx=60)
    Button(root, text='Verify', command=verify_func).grid(row=4, column=3, padx=(5, 10))
    text_box = Text(root, height=7, width=60)
    text_box.grid(row=6, column=0, columnspan=4, pady=(5, 5))
    root.mainloop()
