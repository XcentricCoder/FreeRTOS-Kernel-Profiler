int global = 10;
int uninit;
const int table[4] = {1, 2, 3, 4};

volatile const int *p = table;

int main(void)
{
    int local = 5;

    global++;

    uninit = local;

    local += p[2];

    while (1)
    {
        local++;
    }
}