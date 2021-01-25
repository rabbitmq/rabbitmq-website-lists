public class HotSpotTest {

    public static void main(String[] args) throws Exception {

        int m = Integer.MAX_VALUE;
        int next = 0;
        for (int o = 0; o < 10000; ++o) {

            for (int i = 1; i <= m; ++i) {
                if (i > next) {
                    next = i;
                    break;
                }
            }
        }
        System.out.println("Next=" + next);
        return;
    }
}