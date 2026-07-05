import java.io.*;
import java.nio.file.*;
import java.util.*;
import java.util.concurrent.*;
import java.util.stream.Collectors;

public class BuscadorHtml {
    
    // Configurações gerais
    private static final String NOME_DIRETORIO = "arquivos_html_teste";
    private static final String PALAVRA_ALVO = "tecnologia";

    /**
     * Método utilitário compartilhado que lê um arquivo linha por linha.
     * Retorna true assim que encontra a palavra desejada.
     */
    private static boolean buscarPalavraNoArquivo(Path caminhoArquivo, String palavra) {
        // Usamos BufferedReader para não carregar o arquivo inteiro na RAM de uma vez
        try (BufferedReader reader = Files.newBufferedReader(caminhoArquivo)) {
            String linha;
            while ((linha = reader.readLine()) != null) {
                // Checagem case-insensitive
                if (linha.toLowerCase().contains(palavra.toLowerCase())) {
                    return true;
                }
            }
        } catch (IOException e) {
            System.err.println("Erro ao ler o arquivo: " + caminhoArquivo.getFileName());
        }
        return false;
    }

    /**
     * =================================================================
     * PROGRAMA 1: BUSCA SEQUENCIAL (SEM THREADS)
     * =================================================================
     */
    public static class ProgramaSemThread {
        public static void executar(Path diretorio, String palavra) throws IOException {
            System.out.println("\n[Programa 1] Iniciando busca SEQUENCIAL (Sem threads)...");
            long tempoInicio = System.currentTimeMillis();
            List<String> arquivosEncontrados = new ArrayList<>();

            // Lista todos os arquivos .html no diretório
            List<Path> arquivos = Files.list(diretorio)
                    .filter(p -> p.toString().endsWith(".html"))
                    .collect(Collectors.toList());

            // Laço de repetição sequencial padrão
            for (Path arquivo : arquivos) {
                if (buscarPalavraNoArquivo(arquivo, palavra)) {
                    arquivosEncontrados.add(arquivo.getFileName().toString());
                }
            }

            long tempoFim = System.currentTimeMillis();
            System.out.println(" -> Arquivos que contem a palavra: " + arquivosEncontrados);
            System.out.println(" -> Tempo de execução: " + (tempoFim - tempoInicio) + " ms");
        }
    }

    /**
     * =================================================================
     * PROGRAMA 2: BUSCA PARALELA (COM THREADS)
     * =================================================================
     */
    public static class ProgramaComThread {
        public static void executar(Path diretorio, String palavra) throws IOException, InterruptedException {
            System.out.println("\n[Programa 2] Iniciando busca PARALELA (Com threads)...");
            long tempoInicio = System.currentTimeMillis();
            List<String> arquivosEncontrados = new ArrayList<>();

            // Lista todos os arquivos .html no diretório
            List<Path> arquivos = Files.list(diretorio)
                    .filter(p -> p.toString().endsWith(".html"))
                    .collect(Collectors.toList());

            // Cria um pool de threads otimizado para o número de núcleos de processamento da máquina
            int nucleos = Runtime.getRuntime().availableProcessors();
            System.out.println(" -> Numero de nucleos disponiveis: " + nucleos);
            ExecutorService executor = Executors.newFixedThreadPool(nucleos);

            // Cria uma lista de tarefas (Callable) para serem executadas paralelamente
            List<Callable<String>> tarefas = new ArrayList<>();

            for (Path arquivo : arquivos) {
                tarefas.add(() -> {
                    // Se encontrar a palavra, a thread retorna o nome do arquivo, senão retorna null
                    if (buscarPalavraNoArquivo(arquivo, palavra)) {
                        return arquivo.getFileName().toString();
                    }
                    return null;
                });
            }

            // Executa todas as tarefas simultaneamente e aguarda a finalização
            List<Future<String>> resultados = executor.invokeAll(tarefas);

            // Coleta os resultados das threads
            for (Future<String> resultado : resultados) {
                try {
                    String nomeArquivo = resultado.get();
                    if (nomeArquivo != null) {
                        arquivosEncontrados.add(nomeArquivo);
                    }
                } catch (ExecutionException e) {
                    System.err.println("Erro na execucao de uma thread.");
                }
            }

            // Desliga o pool de threads
            executor.shutdown();

            long tempoFim = System.currentTimeMillis();
            System.out.println(" -> Arquivos que contem a palavra: " + arquivosEncontrados);
            System.out.println(" -> Tempo de execucao: " + (tempoFim - tempoInicio) + " ms");
        }
    }

    /**
     * =================================================================
     * GERADOR DE ARQUIVOS (SETUP)
     * =================================================================
     */
    public static void criarArquivosHtml(Path diretorio, String palavraAlvo) throws IOException {
        System.out.println("Preparando o ambiente: Gerando 200 arquivos HTML locais para o teste...");
        
        if (!Files.exists(diretorio)) {
            Files.createDirectory(diretorio);
        }

        // Criamos 200 arquivos grandes para que a diferença de velocidade das threads seja notável
        for (int i = 1; i <= 200; i++) {
            StringBuilder html = new StringBuilder();
            html.append("<!DOCTYPE html>\n<html lang=\"pt-BR\">\n<head><title>Página ").append(i).append("</title></head>\n<body>\n");

            // Embutir a palavra-alvo a cada 15 arquivos gerados
            if (i % 15 == 0) {
                html.append("<h1>Atenção: A ").append(palavraAlvo).append(" do futuro chegou nesta página!</h1>\n");
            } else {
                html.append("<h1>Página comum de testes sem a palavra desejada.</h1>\n");
            }

            // Adicionando um volume pesado de parágrafos para simular arquivos HTML maiores
            for (int j = 0; j < 1000; j++) {
                html.append("<p>Texto genérico de preenchimento. Esta é a linha de parágrafo número ")
                    .append(j).append(" do arquivo html.</p>\n");
            }
            html.append("</body>\n</html>");

            // Escreve o arquivo no diretório (requer Java 11+)
            Files.writeString(diretorio.resolve("pagina_" + i + ".html"), html.toString());
        }
        System.out.println("Arquivos criados com sucesso na pasta: " + diretorio.toAbsolutePath());
    }

    /**
     * =================================================================
     * MÉTODO PRINCIPAL
     * =================================================================
     */
    public static void main(String[] args) {
        Path diretorio = Paths.get(NOME_DIRETORIO);
        
        try {
            // 1. Gera os arquivos HTML contendo ou não a palavra alvo
            criarArquivosHtml(diretorio, PALAVRA_ALVO);

            // 2. Roda o programa 1
            ProgramaSemThread.executar(diretorio, PALAVRA_ALVO);

            // 3. Roda o programa 2
            ProgramaComThread.executar(diretorio, PALAVRA_ALVO);

        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}