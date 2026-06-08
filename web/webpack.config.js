const path = require("path");
const webpack = require("webpack");

module.exports = (env = {}) => ({
  mode: "production",
  entry: path.resolve(__dirname, "app.mjs"),
  output: {
    path: env.outputPath
      ? path.resolve(env.outputPath)
      : path.resolve(__dirname, "build"),
    filename: "out.js",
    chunkFilename: "chunks/[name]-[contenthash].js",
    clean: true,
  },
  resolve: {
    extensions: [".mjs", ".js"],
    fullySpecified: false,
  },
  module: {
    rules: [
      {
        test: /\.mjs$/,
        type: "javascript/auto",
      },
    ],
  },
  plugins: [
    new webpack.DefinePlugin({
      "process.env.ROSE_EXAMPLES_PATH": JSON.stringify(
        env.examplesPath ?? "examples",
      ),
    }),
  ],
});
