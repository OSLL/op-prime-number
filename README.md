# op-prime-number

### Программа проверки списка чисел на простоту и разложение списка чисел на простые множители.

Для того чтобы собрать программу нужен компилятор gcc не ниже версии 7.3.

Собрать программу можно запустив на исполение Makefile. (команда make в дериетории проекта)

После того как программа будет собрана. Можно будет ее проверить. Для этого
наберите в терминале имя программы (наша программа называется op-prime-number) и укажите следующие параметры:
- Первым обязательным аргкментом которой должен быть путь к файлу, который задается опцией -p [путь к файлу]
  или --path [путь к файлу]. Например: -p './home/numbers' или --path './home/numbers'
- Вторым обязательным аргрументом должен быть 'выходной файл', который задается опцией -o [путь к файлу]
  или --output [путь к файлу]. Например: -o './home/result' или --output './home/result'
- Третьим обязательным аргументом должен быть режим исполнения программы. Есть всего два режима: 1) проверка списка
  чисел на простоту, задаваемый параметром -с  или --check; 2) разложение числа на простые множители(факторизация),
  задаваемая параметром -f или --factor.
- Четвертым необязательным аргументом является опция -s [целое положительное значение: опционально] или
                                                     --scale [целое положительное значение: опционально].
  Если указана опция -s или --scale без значения, то программа автоматически "распараллелит" расчет числел по процессам.
  Если же указана опция -s или --scale со значением n, то будет создано n процессов для обработки.
  На данный момент опция поддерживается, но не реализована реальная поддержка параллелизма так как это первая итерация.

Пример запуска ./op-prime-number -p './home/numbers' -o './home/result' -c


