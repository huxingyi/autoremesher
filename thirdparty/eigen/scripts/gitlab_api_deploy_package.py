"""Helper script to download source archives and upload them to the Eigen GitLab generic package registry."""

import os
import requests
import hashlib
import argparse
import sys
import tempfile

EIGEN_PROJECT_ID = 15462818  # Taken from the gitlab project page.


def calculate_sha256(filepath: str):
    """Calculates the SHA256 checksum of a file."""
    sha256_hash = hashlib.sha256()
    with open(filepath, "rb") as f:
        # Read and update hash in chunks of 4K
        for byte_block in iter(lambda: f.read(4096), b""):
            sha256_hash.update(byte_block)
    return sha256_hash.hexdigest()


def upload_to_generic_registry(
    gitlab_private_token: str, package_name: str, package_version: str, filepath: str
):
    """Uploads a file to the GitLab generic package registry."""
    headers = {"PRIVATE-TOKEN": gitlab_private_token}
    filename = os.path.basename(filepath)
    upload_url = f"https://gitlab.com/api/v4/projects/{EIGEN_PROJECT_ID}/packages/generic/{package_name}/{package_version}/{filename}"

    print(f"Uploading {filename} to {upload_url}...")
    try:
        with open(filepath, "rb") as f:
            response = requests.put(upload_url, headers=headers, data=f)
            response.raise_for_status()
        print(f"Successfully uploaded {filename}.")
        return True
    except requests.exceptions.RequestException as e:
        print(f"Error uploading {filename}: {e}")
        if e.response is not None:
            print(f"Response content: {e.response.text}")
        return False


def main():
    """Main function to download archives and upload them to the registry."""
    parser = argparse.ArgumentParser(
        description="Download GitLab release archives for Eigen and upload them to the generic package registry."
    )
    parser.add_argument(
        "--gitlab_private_token",
        type=str,
        help="GitLab private API token.  Defaults to the GITLAB_PRIVATE_TOKEN environment variable if set.",
    )
    parser.add_argument(
        "--version",
        required=True,
        help="Specify a single version (tag name) to process.",
    )
    parser.add_argument(
        "--download-dir", help=f"Directory to store temporary downloads (optional)."
    )
    args = parser.parse_args()

    if not args.gitlab_private_token:
        args.gitlab_private_token = os.getenv("GITLAB_PRIVATE_TOKEN")
        if not args.gitlab_private_token:
            print("Could not determine GITLAB_PRIVATE_TOKEN.", file=sys.stderr)
            parser.print_usage()
            sys.exit(1)

    # Create download directory if it doesn't exist.
    cleanup_download_dir = False
    if args.download_dir:
        if not os.path.exists(args.download_dir):
            cleanup_download_dir = True
            os.makedirs(args.download_dir)
    else:
        args.download_dir = tempfile.mkdtemp()
        cleanup_download_dir = True

    for ext in ["tar.gz", "tar.bz2", "tar", "zip"]:
        archive_filename = f"eigen-{args.version}.{ext}"
        archive_url = f"https://gitlab.com/libeigen/eigen/-/archive/{args.version}/{archive_filename}"
        archive_filepath = os.path.join(args.download_dir, archive_filename)

        # Download the archive
        print(f"Downloading {archive_url}...")
        try:
            response = requests.get(archive_url, stream=True)
            response.raise_for_status()
            with open(archive_filepath, "wb") as f:
                for chunk in response.iter_content(chunk_size=8192):
                    f.write(chunk)
            print(f"Downloaded to {archive_filepath}")
        except requests.exceptions.RequestException as e:
            print(f"Error downloading {archive_url}: {e}. Skipping.")
            continue

        # Calculate SHA256 sum
        sha256_sum = calculate_sha256(archive_filepath)
        print(f"SHA256 sum: {sha256_sum}")

        # Create SHA256 sum file
        sha_filename = f"{archive_filename}.sha256"
        sha_filepath = os.path.join(args.download_dir, sha_filename)
        with open(sha_filepath, "w") as f:
            f.write(f"{sha256_sum}  {archive_filename}\n")
        print(f"Created SHA256 file: {sha_filepath}")

        # Upload archive to generic registry
        if not upload_to_generic_registry(
            args.gitlab_private_token, "eigen", args.version, archive_filepath
        ):
            # If upload fails, clean up and move to the next release
            os.remove(archive_filepath)
            os.remove(sha_filepath)
            continue

        # Upload SHA256 sum file to generic registry
        upload_to_generic_registry(
            args.gitlab_private_token, "eigen", args.version, sha_filepath
        )

        # Clean up downloaded files
        print("Cleaning up local files...")
        os.remove(archive_filepath)
        os.remove(sha_filepath)

    # Clean up the download directory if it's empty
    if cleanup_download_dir and not os.listdir(args.download_dir):
        os.rmdir(args.download_dir)


if __name__ == "__main__":
    main()
